#include "elf/elf.h"
#include "process.h"
#include "linker.h"
#include "memory/physical.h"
#include "threading/scheduler.h"

UniqueGenerator<u32> process_ids(1);

void test_mem();

void load_binding_and_run(bindings::Binding* bind)
{
    int i = 0;
    ///printf("thread started\n");
    //while (1) {
    //    i = i > 10 ? 1 : i+1;
    //    void* mem = malloc(1000 * i);
        //printf("%x\n", memory::physicalMemoryManager.countFreePhysicalMemory());
    //    if (mem != nullptr)
    //        free(mem);
        //threading::exit();
    //}
    
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

    i32 (*app_main)(int, char**) = 0;

    {
        // link it
        elf::elf e(buffer, false);
        if (e.link_in_userspace() == 0) {
            e.get_symbol_value("main", (u32*) &app_main);
        } else {
            printf("failed linking elf\n");
        }

        delete buffer;
    }

    threading::currentThread()->process()->status_code = 0;//app_main(110, NULL);

    //printf("childs: %d\n", threading::currentThread()->process()->childs.size());

    /*for (size_t i = 0 ; i < threading::currentThread()->process()->childs.size() ; i++) {
        threading::currentThread()->process()->childs[i]->thread->kill();
    }*/

    /*for (size_t i = 0 ; i < childs.size() ; i++) {
        while (!childs[i]->thread->finished())
            threading::exit();
    }*/

    threading::currentThread()->process()->state = ProcessState::Stopped;
}

Process::Process(): thread(nullptr), _pagetable(nullptr), _binding_ids(1), status_code(-1), pid(process_ids.next()), _parent(NULL), state(ProcessState::Initing), childs()
{
}

Process::~Process()
{
    if (_pagetable) {
       memory::freePageDirectory(_pagetable);
    }

    if (thread) {
       delete thread;
    }

    for (size_t i = 0 ; i < childs.size() ; i++) {
        while (!childs[i]->thread->finished())
            threading::exit();
    }

    for (size_t i = 0 ; i < childs.size() ; i++) {
        delete childs[i];
    }
}

void Process::init(bindings::Binding* file)
{
    // Create a new pagetable for the process

    bool eflag = CLI();
    memory::PageDirectory* current = memory::PageDirectory::current();
    ((memory::PageDirectory*)(current->getPhysicalAddress(&memory::kernelPageDirectory)))->use();
    _pagetable = memory::kernelPageDirectory.clone();

    // Allocate a stack in the process memory itself
    memory::PageDirectory* kpage = ((memory::PageDirectory*)(current->getPhysicalAddress(&memory::kernelPageDirectory)));
    if (current != kpage) {
        CPU::panic("current not equal to kernel\n");
    }

	((memory::PageDirectory*)(current->getPhysicalAddress(_pagetable)))->use();
    /*char* stackStart = (char*) (0xC0000000 - THREAD_STACK_SIZE - 0x10000);
    for (size_t i = 0 ; i < THREAD_STACK_SIZE ; i += 0x1000) {
        current->bindVirtualPage(stackStart + i);
    }*/
    char* stackStart = (char*) current->bindFirstFreeVirtualPages((char*) 0xC0000000 - THREAD_STACK_SIZE, THREAD_STACK_SIZE / 0x1000);
    printf("stack: %x\n", stackStart );
    if (stackStart != (char*)0xbfff0000) {
        printf("corrupt stack\n");
        memory::PageDirectory* current = memory::PageDirectory::current();
        ((memory::PageDirectory*)(current->getPhysicalAddress(&memory::kernelPageDirectory)))->use();
        while(1);
    }

    // And create a thread that will init the process
    thread = new threading::Thread(this, stackStart, load_binding_and_run, file);
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
    while (!thread->finished()) {
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

    return len;
}

i32 Process::read(i32 file, char* data, i32 len)
{
    if (_bindings.count(file) == 0) {
        return -1;
    }

    BindingDescriptor& desc = _bindings[file];
    size_t status = desc.binding->read(data, len, desc.offset);
    desc.offset += status;

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
    return;
    state = ProcessState::Running;

    Process* child = new Process();
    
    child->_parent = this;
    child->_pagetable = clone;
    child->_bindings = _bindings;
    child->_binding_ids = _binding_ids;
    child->thread = thread->clone();
    child->thread->_process = child;

    childs.push_back(child);
    
    printf("adding child\n");
    
    threading::scheduler->schedule(child->thread);
}

i32 Process::fork()
{
    state = ProcessState::Forking;
    threading::exit();

    if (threading::currentThread()->process()->pid == pid) {
        return childs[childs.size() - 1]->pid;
    } else {
        return 0;
    }
}