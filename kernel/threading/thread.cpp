#include "cdefs.h"
#include "threading/thread.h"
#include "apic.h"

using namespace threading;

extern "C" u32 __attribute__ ((noinline)) thread_enter(volatile u32* pEsp, volatile u32* esp);
extern "C" void __attribute__ ((noinline)) thread_exit(volatile u32* pEsp);

// A generator to create unique pids for each thread
UniqueGenerator<u32> threading::pidGenerator;

// global values that hold the parent stack pointer of a running thread. Each core will always use his own index.
u32 parent_stack_pointers[MAX_CORE_COUNT];
bool in_thread[MAX_CORE_COUNT];

Thread::Thread(void(*entry)()): _finished(false), _id(pidGenerator.next()), _blocking(false) {
    // A new thread allocates his own stack
    _stack = new char[THREAD_STACK_SIZE];
    memset(_stack, 0, THREAD_STACK_SIZE);

    // we first place the entry address, then 36 bytes zero (pushad + pushfd) but those are already zero with memset
    *(u32*)(_stack + THREAD_STACK_SIZE - 8) = (u32)entry;
    esp = (u32)(_stack + THREAD_STACK_SIZE - 44);
}

Thread::Thread(Thread& thread) {
    // Clone the stack of the thread we are copying
    _stack = new char[THREAD_STACK_SIZE];
    memcpy(_stack, thread._stack, THREAD_STACK_SIZE);
    esp = thread.esp;
    _finished = thread._finished;
    _id = thread._id;
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
}

Thread::~Thread() {
    if (_stack)
        delete[] _stack;
}

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
}

void Thread::enter() {
    if (!_finished) {
        // whenever we enter the thread, we set the very first item on the stack to our parent stack pointer.
        // this way the child stack can access it when it finishes on its own.
        CLI();
        
        in_thread[apic::id()] = true;
        volatile u32* parent_pointer = parent_stack_pointers + apic::id();
        *(volatile u32*)(_stack + THREAD_STACK_SIZE - 4) = (u32)parent_pointer;
        volatile u32 status = thread_enter(parent_pointer, &esp);
        if (status == 0)
            _finished = true;
        in_thread[apic::id()] = false;

        STI();
    }
}

u32 Thread::pid() {
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

void threading::exit() {
    CLI();
    if (is_current_core_in_thread()) {
        // if we exit a thread, the thread_exit function will enable interrupts again
        thread_exit(parent_stack_pointers + apic::id());
    }
}
// Returns true if the given physical core is currently running a thread
bool threading::is_core_in_thread(u8 core) {
    return in_thread[core];
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