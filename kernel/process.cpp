#include "elf/elf.h"
#include "process.h"
#include "threading/scheduler.h"

Process::Process(): _thread(nullptr), _pagetable(nullptr), _binding_ids(1)
{
}

Process::~Process()
{
    if (_pagetable)
        memory::freePageDirectory(_pagetable);
    if (_thread) {
        delete _thread;
    }
}

void Process::init(bindings::Binding* file)
{
    _pagetable = memory::kernelPageDirectory.clone();

    // And create a thread that will init the process
    _thread = new threading::Thread(this, &Process::load_binding_and_run, this, file);
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

memory::PageDirectory* Process::pagetable()
{
    return _pagetable;
}

void Process::load_binding_and_run(bindings::Binding* bind)
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

    // link it
    elf::elf e(buffer, false);

	if (e.link_in_userspace() == 0) {
        delete buffer;
        // and execute the main
		void (*app_main)(int, char**) = 0;
		e.get_symbol_value("main", (u32*) &app_main);
		app_main(110, NULL);
	} else {
        delete buffer;
		printf("failed linking elf\n");
	}
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