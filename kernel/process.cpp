#include "elf/elf.h"
#include "process.h"
#include "linker.h"
#include "cpu/hpet.h"
#include "kstd/shared_ptr.h"
#include "memory/physical.h"
#include "threading/thread.h"
#include "threading/scheduler.h"
#include "filesystem/stream.h"
#include "filesystem/virtualfolder.h"
#include <string.h>

extern "C" {
    #include <sys/types.h>
    #include <unistd.h>
    #include <sys/wait.h>
}

#define MAX_ARGS 128
#define MAX_ARGS_LENGTH 2048

UniqueGenerator<u32> process_ids(1);


extern "C" void jump_usermode(void* func, int argc, char** argv, char** env);

void load_file_and_run(fs::File* file, std::vector<std::string>* args)
{
    // Get the filesize of the bind
    fs::FileHandle* handle = file->open();

	size_t filesize = handle->get_size();
	if (filesize == 0) {
		printf("Filesize is 0.\n");
		return;
	}

    // Fully load the binary in kernel memory
    if (handle == nullptr) {
        return;
    }

	u8* buffer = new u8[filesize];

	size_t total = 0;
	do {
		size_t read = handle->read((void*)(buffer + total), filesize - total < 4096 ? filesize - total : 4096, total);
		total += read;
	} while (total != filesize);

    handle->close();

    void (*app_entry)(int argc, char** arv, char** environ) = 0;
    {
        // link it
        elf::elf e(buffer, false);
        if (e.link_in_userspace() == 0) {
            if (e.get_symbol_value("_start", (u32*) &app_entry) != 0) {
                printf("Failed fetching _start\n");
            }
        } else {
            printf("failed linking elf\n");
        }

        threading::currentThread()->process->set_program_break((char*) e.get_program_break());

        delete buffer;
    }

    // Copy the arguments on the stack
    int argdc = 0;
    char* argv[MAX_ARGS];
    char argd[MAX_ARGS_LENGTH];
    for (size_t i = 0 ; i < args->size() ; i++) {
        argv[i] = argd + argdc;
        strcpy(argd + argdc, args->at(i).c_str());
        argdc += args->at(i).length() + 1;
    }

    int argc = args->size();

    // Run the entry point of the userspace application
    jump_usermode((void*) *app_entry, argc, argv, nullptr);
}

Process::Process(): status_code(-1), state(ProcessState::Initing), thread(nullptr), childs(), pid(process_ids.next()), _pagetable(nullptr), _bindings(1024), _descriptor(nullptr), _waitingForStopped(), _waitingForChildStopped(), _stateLock(), _workingDirectory("")
{
    // Make a binding for the process
    char name[32];
    sprintf(name, "%d", pid);
    _descriptor = new fs::VirtualFolder(name);
}

Process::~Process()
{
    for (size_t i = 0 ; i < childs.size() ; i++) {
        childs[i]->thread->kill();
    }

    if (_pagetable) {
       memory::freePageDirectory(_pagetable);
    }

    delete thread;

    for (size_t i = 0 ; i < childs.size() ; i++) {
        childs[i]->wait();
        delete childs[i];
    }
}

void Process::set_arguments(std::vector<std::string>& args)
{
    _args = args;
}

void Process::init(fs::File* file, std::string& workingDirectory)
{
    _file = file;
    _workingDirectory = workingDirectory;

    bool eflag = CLI();

    // Create a new pagetable for the process
    memory::PageDirectory* current = memory::PageDirectory::current();
    ((memory::PageDirectory*)(current->getPhysicalAddress(&memory::kernelPageDirectory)))->use();
    _pagetable = memory::kernelPageDirectory.clone();

    // Allocate a stack in the process memory itself
    memory::PageDirectory* kpage = ((memory::PageDirectory*)(current->getPhysicalAddress(&memory::kernelPageDirectory)));
    if (current != kpage) {
        CPU::panic("current not equal to kernel\n");
    }

	((memory::PageDirectory*)(current->getPhysicalAddress(_pagetable)))->use();

    // allocate a normal stack
    char* stackStart = (char*) current->bindFirstFreeVirtualPages((char*) 0xC0000000 - THREAD_STACK_SIZE, THREAD_STACK_SIZE / 0x1000, memory::UserMode::User);

    // allocate a kernel stack
    kernel_stack = (char*) current->bindFirstFreeVirtualPages((char*) 0x20000000 - PROCESS_KERNEL_STACK_SIZE, PROCESS_KERNEL_STACK_SIZE / 0x1000, memory::UserMode::Supervisor);


    if (stackStart != (char*)0xbfff0000) {
        CPU::panic("corrupt stack\n");
        memory::PageDirectory* current = memory::PageDirectory::current();
        ((memory::PageDirectory*)(current->getPhysicalAddress(&memory::kernelPageDirectory)))->use();
        while(1);
    }

    // And create a thread that will init the process
    thread = new threading::Thread(this, stackStart, load_file_and_run, _file, &_args);
    ((memory::PageDirectory*)(_pagetable->getPhysicalAddress(current)))->use();
    state = ProcessState::Running;

    STI(eflag);
}

void Process::start()
{
    threading::scheduler->schedule(thread);
}

void Process::wait()
{
    bool eflags = CLI();
    _stateLock.lock();

    if (state != ProcessState::Stopped) {
        _waitingForStopped.add_blocked_thread(threading::currentThread());
        _stateLock.release();
        STI(eflags);
        threading::exit();
    } else {        
        _stateLock.release();
        STI(eflags);
    }    
}

memory::PageDirectory* Process::pagetable()
{
    return _pagetable;
}

void Process::set_program_break(char* program_break)
{
    _program_break = program_break;
}

void Process::set_state(ProcessState new_state)
{
    bool eflags = CLI();
    _stateLock.lock();

    state = new_state;
    if (state == ProcessState::Stopped) {
        _waitingForStopped.unblock_all_threads();
        if (_parent) {
            _parent->_stateLock.lock();
            if (_parent->state == ProcessState::Waiting) {
                _parent->_waitingForChildStopped.unblock_all_threads();
            }
            _parent->_stateLock.release();
        }
    }
    _stateLock.release();
    STI(eflags);
}

i32 Process::open(const char* name, i32 flags, i32 mode)
{
    fs::File* wd = fs::root->get(_workingDirectory.c_str());
    if (!wd) {
        return -1;
    }

    return open(wd->get(name), flags, mode);

}

i32 Process::open(fs::File* f, i32 flags, i32 mode)
{
    UNUSED(flags);
    UNUSED(mode);
    if (f == 0) {
        return -1;
    }

    fs::FileHandle* handle = f->open();
    if (handle == nullptr) {
        return -1;
    }

    i32 id = _bindings.new_resource();
    if (id < 0) {
        handle->close();
        return -1;
    }

    std::shared_ptr<FileDescriptor>* desc = _bindings.at(id);
    *desc = new FileDescriptor(handle);

    return id;
}

i32 Process::close(i32 file)
{
    std::shared_ptr<FileDescriptor>* desc = _bindings.at(file);
    if (desc == nullptr) {
        return -1;
    }

    _bindings.free_resource(file);

    return 0;
}

i32 Process::write(i32 file, const char* data, i32 len)
{
    std::shared_ptr<FileDescriptor>* desc = _bindings.at(file);
    if (desc == nullptr) {
        return -1;
    }

    i32 ret = (*desc)->handle->write(data, len, (*desc)->offset);
    (*desc)->offset += ret;

    return ret;
}

i32 Process::read(i32 file, char* data, i32 len)
{
    std::shared_ptr<FileDescriptor>* desc = _bindings.at(file);
    if (desc == nullptr) {
        return -1;
    }

    i32 status = (*desc)->handle->read(data, len, (*desc)->offset);
    (*desc)->offset += status;

    return status;
}

i32 Process::exit(i32 status)
{
    status_code = status;
    thread->kill();
    threading::exit();

    return 0;
}

void Process::finish_fork(memory::PageDirectory* clone)
{
    state = ProcessState::Running;
    Process* child = new Process;

    //child->_parent = this;
    child->_pagetable = clone;
    child->_file = _file;
    child->_args = _args;
    child->_bindings = _bindings;
    child->_program_break = _program_break;
    child->state = state;
    child->_parent = this;
    child->thread = thread->clone();
    child->thread->process = child;
    child->_workingDirectory = _workingDirectory;
    child->kernel_stack = kernel_stack;
    childs.push_back(child);

    threading::scheduler->schedule(child->thread);
}

void Process::finish_execve()
{        
    // Destroy old process state
    if (_pagetable) {
       memory::freePageDirectory(_pagetable);
    }

    // Reinitialize process and start
    init(_file, _workingDirectory);
    start();
}

i32 Process::fork()
{
    state = ProcessState::Forking;
    threading::exit();

    if (threading::currentThread()->process->pid == pid) {
        return childs[childs.size() - 1]->pid;
    } else {
        return 0;
    }
}

i32 Process::execve(const char* pathname, char *const *argv, char *const *envp)
{
    fs::File* wd = fs::root->get(_workingDirectory.c_str());
    if (!wd) {
        return -1;
    }

    fs::File* child = wd->get(pathname);
    if (!child) {
        return -1;
    }

    state = ProcessState::Execve;

    // Copy the new process arguments to this process
    _file = child;
    _args.clear();
    _args.push_back(pathname);
    size_t i = 0;
    while (argv[i] != nullptr) {
        _args.push_back(argv[i]);
        i++;
    }
    
    // TODO copy environment
    threading::exit();
    return -2;
}

i32 Process::wait(i32* status)
{
    while (true)
    {
        bool eflags = CLI();
        _stateLock.lock();
        //_state = ProcessState::Waiting;

        if (childs.size() > 0) {
            for (size_t i = 0 ; i < childs.size() ; i++) {
                if (childs[i]->state == ProcessState::Stopped) {
                    i32 pid = childs[i]->pid;
                    Process* child = childs[i];
                    childs.erase(i);
                    child->state = ProcessState::PendingDestruction;
                    threading::scheduler->schedule(child->thread);

                    if (status) {
                        *status = child->status_code;
                    }

                    state = ProcessState::Running;
                    _stateLock.release();
                    STI(eflags);
                    return pid;
                }
            }
            
            state = ProcessState::Waiting;
            _waitingForChildStopped.add_blocked_thread(threading::currentThread());
            _stateLock.release();
            STI(eflags);
            threading::exit();
        } else {
            return -1;
        }
    }
}

i32 Process::isatty(i32 file)
{
    UNUSED(file);
    return 1;
}

i32 Process::lseek(i32 file, i32 ptr, i32 dir)
{
    std::shared_ptr<FileDescriptor>* desc = _bindings.at(file);
    if (desc == nullptr) {
        return -1;
    }
    
    if (dir == SEEK_SET) {
        (*desc)->offset = ptr;
    } else if (dir == SEEK_CUR) {
        (*desc)->offset += ptr;
    } else if (dir == SEEK_END) {
        (*desc)->offset = (*desc)->handle->get_size() + ptr;
    }

    return (*desc)->offset;
}

i32 Process::fstat(i32 file, struct stat* st)
{
    UNUSED(file);
    UNUSED(st);
    return 1;
}

void* Process::sbrk(i32 inc)
{
    if (inc == 0) {
        return _program_break;
    } else {
        char* cur_brk = _program_break;
        char* new_brk = _program_break + inc;

        size_t cur_brk_page = (u32)(cur_brk) / 0x1000;
        size_t new_brk_page = (u32)(new_brk) / 0x1000;

        for (size_t i = cur_brk_page ; i < new_brk_page ; i++) {
            memory::PageDirectory::current()->bindVirtualPage((void*)((i+1) * 0x1000), memory::UserMode::User);
        }

        _program_break = new_brk;
        return cur_brk;
    }
}

i32 Process::pipe(i32 pipefd[2])
{
    i32 readId = _bindings.new_resource();
    if (readId < 0) {
        return -1;
    }

    i32 writeId = _bindings.new_resource();
    if (writeId < 0) {
        _bindings.free_resource(readId);
        return -1;
    }

    std::shared_ptr<fs::Stream> stream(new fs::Stream("", 512));
    fs::OwnedStreamHandle* readHandle  = new fs::OwnedStreamHandle(stream, fs::OwnedStreamHandle::WriteMode::Read);
    fs::OwnedStreamHandle* writeHandle = new fs::OwnedStreamHandle(stream, fs::OwnedStreamHandle::WriteMode::Write);

    std::shared_ptr<FileDescriptor>* readDesc  = _bindings.at(readId);
    std::shared_ptr<FileDescriptor>* writeDesc = _bindings.at(writeId);
    *readDesc  = new FileDescriptor(readHandle);
    *writeDesc = new FileDescriptor(writeHandle);

    pipefd[0] = readId;
    pipefd[1] = writeId;

    return 0;
}

i32 Process::dup(int filedes)
{
    std::shared_ptr<FileDescriptor>* olddesc = _bindings.at(filedes);
    if (olddesc == nullptr) {
        return -1;
    }

    i32 filedes2 = _bindings.new_resource();
    if (filedes2 < 0) {
        return -1;
    }
    std::shared_ptr<FileDescriptor>* newdesc = _bindings.at(filedes2);

    *newdesc = *olddesc;

    return filedes2;
}

i32 Process::dup2(int filedes, int filedes2)
{
    std::shared_ptr<FileDescriptor>* olddesc = _bindings.at(filedes);
    if (olddesc == nullptr) {
        return -1;
    }

    std::shared_ptr<FileDescriptor>* newdesc = _bindings.at(filedes2);
    if (newdesc != nullptr) {
        close(filedes2);
    }
    newdesc = _bindings.new_resource(filedes2);
    
    *newdesc = *olddesc;

    return filedes2;
}

i32 Process::readdir(i32 filedes, struct dirent* dirent)
{
    std::shared_ptr<FileDescriptor>* desc = _bindings.at(filedes);
    if (desc == nullptr) {
        return -1;
    }

    fs::File* child = (*desc)->handle->next_child();

    if (child == 0) {
        return -1;
    } else {
        dirent->d_ino = 0;

        const char* child_name = child->get_name();
        int length = strlen(child_name);
        if (length >= NAME_MAX) {
            length = NAME_MAX - 1;
        }

        memcpy(dirent->d_name, child_name, length);
        dirent->d_name[length] = 0;
        return 0;
    }
}

char* Process::getwd(char* buf, size_t size)
{
    size_t length = _workingDirectory.length();

    if (length >= size) {
        return NULL;   
    } else {
        strcpy(buf, _workingDirectory.c_str());
        return buf;
    }
}


i32 Process::usleep(u32 microseconds)
{
    thread->setBlocking(true);

    hpet::hpet.wait<Process>(microseconds, [](Process* p) {
        p->thread->setBlocking(false);
        threading::scheduler->schedule(p->thread);
    }, this);

    threading::exit();

    return 0;
}
