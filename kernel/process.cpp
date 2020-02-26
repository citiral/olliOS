#include "elf/elf.h"
#include "process.h"
#include "linker.h"
#include "memory/physical.h"
#include "threading/scheduler.h"
#include "kstd/shared_ptr.h"
#include "threading/thread.h"

#define MAX_ARGS 128
#define MAX_ARGS_LENGTH 2048

UniqueGenerator<u32> process_ids(1);

void load_binding_and_run(bindings::Binding* bind, std::vector<std::string>* args)
{
    // Get the filesize of the bind
	size_t filesize = bind->get_size();
	if (filesize == 0) {
		printf("Filesize is 0.\n");
		return;
	}

    // Fully load the binary in kernel memory
	u8* buffer = new u8[filesize];

	size_t total = 0;
	do {
		size_t read = bind->read((void*)(buffer + total), filesize - total, total);
		total += read;
	} while (total != filesize);

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

    // Run the entry point of the userspace application
    app_entry(args->size(), argv, nullptr);
}

Process::Process(): status_code(-1), state(ProcessState::Initing), thread(nullptr), childs(), pid(process_ids.next()), _binding_ids(1), _pagetable(nullptr)
{
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

void Process::init(bindings::Binding* file, std::vector<std::string> args)
{
    _args = args;
    _file = file;

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

    char* stackStart = (char*) current->bindFirstFreeVirtualPages((char*) 0xC0000000 - THREAD_STACK_SIZE, THREAD_STACK_SIZE / 0x1000);

    if (stackStart != (char*)0xbfff0000) {
        printf("corrupt stack\n");
        memory::PageDirectory* current = memory::PageDirectory::current();
        ((memory::PageDirectory*)(current->getPhysicalAddress(&memory::kernelPageDirectory)))->use();
        while(1);
    }

    // And create a thread that will init the process
    thread = new threading::Thread(this, stackStart, load_binding_and_run, _file, &_args);
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
    while (state != ProcessState::Stopped) {
        threading::exit();
    }
}

memory::PageDirectory* Process::pagetable()
{
    return _pagetable;
}

i32 Process::open(const char* name, i32 flags, i32 mode)
{
    bindings::Binding* f = bindings::root->get(name);

    if (f == nullptr) {
        return -1;
    }

    i32 id = _binding_ids.next();

    BindingDescriptor& desc = _bindings[id];
    desc.binding = f;
    desc.offset = 0;

    return id;
}

i32 Process::close(i32 file)
{
    if (_bindings.count(file) == 0) {
        return -1;
    }

    _bindings.erase(file);

    return 0;
}

i32 Process::write(i32 file, char* data, i32 len)
{
    if (_bindings.count(file) == 0) {
        return -1;
    }

    BindingDescriptor& desc = _bindings[file];
    desc.binding->write(data, len);

    return 0;
}

i32 Process::read(i32 file, char* data, i32 len)
{
    if (_bindings.count(file) == 0) {
        return -1;
    }

    BindingDescriptor& desc = _bindings[file];
    size_t status = desc.binding->read(data, len, desc.offset);
    desc.offset += status;

    return 0;
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
    child->_binding_ids = _binding_ids;
    child->state = state;
    child->thread = thread->clone();
    child->thread->process = child;
    childs.push_back(child);
    
    threading::scheduler->schedule(child->thread);
}

void Process::finish_execve()
{    
    // Destroy old process state
    if (_pagetable) {
       memory::freePageDirectory(_pagetable);
    }

    delete thread;

    // Reinitialize process and start
    init(_file, _args);
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
    bindings::Binding* child =  _file->_parent->get(pathname);
    if (!child) {
        return -1;
    }

    state = ProcessState::Execve;

    // Copy the new process arguments to this process
    _file = child;
    _args.clear();
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
    while (childs.size() > 0) {
        for (size_t i = 0 ; i < childs.size() ; i++) {
            if (childs[i]->state == ProcessState::Stopped) {
                i32 pid = childs[i]->pid;
                Process* child = childs[i];
                childs.erase(i);
                child->state = ProcessState::PendingDestruction;
                threading::scheduler->schedule(child->thread);
                return pid;
            }
        }

        threading::exit();
    }

    return -1;
}

i32 Process::isatty(i32 file)
{
    return 1;
}