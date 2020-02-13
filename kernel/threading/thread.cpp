#include "threading/thread.h"
#include "cpu/apic.h"
#include "cdefs.h"
#include "process.h"

using namespace threading;

// A generator to create unique pids for each thread
UniqueGenerator<u32> threading::pidGenerator;

// global values that hold the parent stack pointer of a running thread. Each core will always use his own index.
u32 parent_stack_pointers[MAX_CORE_COUNT];
Thread* running_thread[MAX_CORE_COUNT];
/*
Thread::Thread(Thread& thread) {
    // Clone the stack of the thread we are copying
    _stack = new char[THREAD_STACK_SIZE];
    memcpy(_stack, thread._stack, THREAD_STACK_SIZE);
    esp = thread.esp;
    _finished = thread._finished;
    _id = pidGenerator.next();
    _blocking = thread._blocking;
}

Thread::Thread(Thread&& thread) {
    // Swap our stack with the moved stack
    _stack = thread._stack;
    thread._stack = nullptr;
    esp = thread.esp;
    _finished = thread._finished;
    _id = thread._id;
    _blocking = thread._blocking;
}*/

Thread::~Thread() {
    if (_stack && _ownsStack) {
        delete[] _stack;
    }
}
/*
Thread& Thread::operator=(Thread& t) {
    if (_stack)
        delete[] _stack;
    
    // Clone the stack of the thread we are copying
    _stack = new char[THREAD_STACK_SIZE];
    memcpy(_stack, t._stack, THREAD_STACK_SIZE);
    esp = t.esp;
    _finished = t._finished;
    _id = t._id;
    _blocking = t._blocking;

    return *this;
}

Thread& Thread::operator=(Thread&& t) {
    char* tmp = _stack;
    _stack = t._stack;
    t._stack = tmp;
    esp = t.esp;
    _finished = t._finished;
    _id = t._id;
    _blocking = t._blocking;

    return *this;
}*/


Thread* Thread::clone() {
    Thread* child = new Thread();

    child->esp = esp;
    child->_finished = _finished;
    child->_id = pidGenerator.next();
    child->_blocking = _blocking;
    child->_process = _process;
    child->_ownsStack = _ownsStack;
    child->_stack = _stack;

    return child;
}

void Thread::enter() {
    if (!_finished) {
        // whenever we enter the thread, we set the very first item on the stack to our parent stack pointer.
        // this way the child stack can access it when it finishes on its own.
        bool eflag = CLI();

        // keep track that we are in a thread
        running_thread[apic::id()] = this;

        // If the thread has a process, and it is forking, do that first!
        if (_process && _process->state() == ProcessState::Forking) {            
            ((memory::PageDirectory*)memory::kernelPageDirectory.getPhysicalAddress(_process->pagetable()))->use();
            memory::PageDirectory* clone = _process->pagetable()->deep_clone();
            ((memory::PageDirectory*)clone->getPhysicalAddress(&memory::kernelPageDirectory))->use();
            _process->finish_fork(clone);
        }

        // load the thread's pagetable
        if (_process)
	        ((memory::PageDirectory*)(memory::kernelPageDirectory.getPhysicalAddress(_process->pagetable())))->use();

        // prepare the thread stack for entering
        volatile u32* parent_pointer = parent_stack_pointers + apic::id();
        *(volatile u32*)(_stack + THREAD_STACK_SIZE - 4) = (u32)parent_pointer;

        // enter the thread
        volatile u32 status = thread_enter(parent_pointer, &esp);

        // load the thread's pagetable
        if (_process)
	        ((memory::PageDirectory*)(memory::kernelPageDirectory.getPhysicalAddress(&memory::kernelPageDirectory)))->use();

        // check if the thread quit because it finished
        if (status == 0)
            _finished = true;

        // keep track that we are not in a thread anymore
        running_thread[apic::id()] = nullptr;

        STI(eflag);
    }
}

u32 Thread::id() {
    return _id;
}

bool Thread::finished() {
    return _finished;
}

bool Thread::blocking() {
    return _blocking;
}

void Thread::setBlocking(bool blocking) {
    _blocking = blocking;
}

Process* Thread::process()
{
    return _process;
}

void Thread::kill() {
    _finished = true;
}

void threading::exit() {
    CLI();
    if (is_current_core_in_thread()) {
        // if we exit a thread, the thread_exit function will enable interrupts again
        thread_exit(parent_stack_pointers + apic::id());
    }
}

// Returns true if the given physical core is currently running a thread
bool threading::is_core_in_thread(u8 core) {
    return running_thread[core] != nullptr;
}

extern "C" u32* __attribute__ ((noinline)) get_parent_stack() {
    return parent_stack_pointers + apic::id();
}

// Returns true if the calling physical core is currently running a thread
bool threading::is_current_core_in_thread() {
    return is_core_in_thread(apic::id());
}

extern "C" bool __attribute__ ((noinline)) is_current_core_in_thread() {
    return threading::is_current_core_in_thread();
}

Thread* threading::currentThread() {
    return running_thread[apic::id()];
}