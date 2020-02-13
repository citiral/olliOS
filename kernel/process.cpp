#include "elf/elf.h"
#include "process.h"
#include "linker.h"
#include "threading/scheduler.h"

UniqueGenerator<u32> process_ids(1);

Process::Process(): _thread(nullptr), _pagetable(nullptr), _binding_ids(1), _status_code(-1), _pid(process_ids.next()), _parent(NULL), _state(ProcessState::Initing)
{
}

Process::~Process()
{
    if (_pagetable) {
        memory::freePageDirectory(_pagetable);
    }
    if (_thread) {
        delete _thread;
    }
}

void Process::init(bindings::Binding* file)
{
    // Create a new pagetable for the process
    _pagetable = memory::kernelPageDirectory.clone();

    bool eflag = CLI();

    // Allocate a stack in the process memory itself
    memory::PageDirectory* current = memory::PageDirectory::current();
	((memory::PageDirectory*)(current->getPhysicalAddress(_pagetable)))->use();
    char* stackStart = (char*) (0xC0000000 - THREAD_STACK_SIZE);
    for (size_t i = 0 ; i < THREAD_STACK_SIZE ; i += 0x1000) {
        current->bindVirtualPage(stackStart + i);
    }

    // And create a thread that will init the process
    _thread = new threading::Thread(this, stackStart, &Process::load_binding_and_run, this, file);
    ((memory::PageDirectory*)(_pagetable->getPhysicalAddress(current)))->use();
    _state = ProcessState::Running;

    STI(eflag);
}

void Process::start()
{
    threading::scheduler->schedule(_thread);
}

void Process::wait()
{
    while (!_thread->finished()) {
        threading::exit();
    }
}

i32 Process::status_code()
{
    return _status_code;
}

memory::PageDirectory* Process::pagetable()
{
    return _pagetable;
}

ProcessState Process::state()
{
    return _state;
}

void Process::load_binding_and_run(bindings::Binding* bind)
{
    printf("thread started\n");
    
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
    
        printf("running main %x -> %x\n", &app_main, app_main);
    _status_code = app_main(110, NULL);
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
    _status_code = status;
    _thread->kill();
    threading::exit();

    return 0;
}

void cloneHelper(memory::PageDirectory* parent, memory::PageDirectory** child)
{
    bool eflag = CLI();
	((memory::PageDirectory*)memory::PageDirectory::current()->getPhysicalAddress(parent))->use();
    memory::PageDirectory* clone = parent->deep_clone();
    *child = clone;
	((memory::PageDirectory*)clone->getPhysicalAddress(&memory::kernelPageDirectory))->use();
    STI(eflag);
    
    return;
}

void Process::finish_fork(memory::PageDirectory* clone)
{
    _state = ProcessState::Running;

    Process* child = new Process();
    
    child->_parent = this;
    child->_pagetable = clone;
    child->_bindings = _bindings;
    child->_binding_ids = _binding_ids;
    child->_thread = _thread->clone();
    child->_thread->_process = child;
    
    threading::scheduler->schedule(child->_thread);
}

i32 Process::fork()
{
    _state = ProcessState::Forking;
    threading::exit();

    if (threading::currentThread()->process()->_pid == _pid) {
        /*bool eflag = CLI();
        printf("finishing: %d\n", threading::currentThread()->process()->_pid);
        printf("hello from parent\n");
        STI(eflag);*/
        return 1;
    } else {
        /*bool eflag = CLI();
        printf("finishing: %d\n", threading::currentThread()->process()->_pid);
        printf("HELLO FROM CHILD!\n");
        STI(eflag);*/
        return 0;
    }
}