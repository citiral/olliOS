#include "thread.h"
#include "apic.h"
#include <string.h>

using namespace threading;

extern "C" volatile u32 thread_enter(u32* pEsp, u32* esp);
extern "C" volatile void thread_exit(u32* pEsp);

// global values that hold the parent stack pointer of a running thread. Each core will always use his own index.
// 16 is enough to hold all possible processors.
u32 parent_stack_pointers[16];

Thread::Thread(void(*entry)()): _finished(false) {
    // A new thread allocates his own stack
    _stack = new char[THREAD_STACK_SIZE];
    memset(_stack, 0, THREAD_STACK_SIZE);

    // we first place the entry address, then 32 bytes zero (pushad) but those are already zero with memset
    *(u32*)(_stack + THREAD_STACK_SIZE - 8) = (u32)entry;
    esp = (u32)(_stack + THREAD_STACK_SIZE - 40);
}

Thread::Thread(Thread& thread) {
    // Clone the stack of the thread we are copying
    _stack = new char[THREAD_STACK_SIZE];
    memcpy(_stack, thread._stack, THREAD_STACK_SIZE);
    esp = thread.esp;
    _finished = thread._finished;
}

Thread::Thread(Thread&& thread) {
    // Swap our stack with the moved stack
    _stack = thread._stack;
    thread._stack = nullptr;
    esp = thread.esp;
    _finished = thread._finished;
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

    return *this;
}

Thread& Thread::operator=(Thread&& t) {
    char* tmp = _stack;
    _stack = t._stack;
    t._stack = tmp;
    esp = t.esp;
    _finished = t._finished;

    return *this;
}

void Thread::enter() {
    if (!_finished) {
        // whenever we enter the thread, we set the very first item on the stack to our parent stack pointer.
        // this way the child stack can access it when it finishes on its own.
        u32* parent_pointer = parent_stack_pointers + apic::id();
        *(u32*)(_stack + THREAD_STACK_SIZE - 4) = (u32)parent_pointer;
        u32 status = thread_enter(parent_pointer, &esp);
        if (status == 0)
            _finished = true;
    }
}

void threading::exit() {
    thread_exit(parent_stack_pointers + apic::id());
}