#ifndef __THREAD_H_
#define __THREAD_H_

#include <types.h>

namespace threading {
    // We define a thread's stack size to be 16kb, which should be more than enough.
    // TODO some thread protected mechanisms, through paging perhaps?
    #define THREAD_STACK_SIZE 0x1000*16

    // A threadstate represents all data the cpu needs to resume a task (the registers, stack, pagetable, ...)
    struct ThreadState {

    };

    // A thread manages a single running function, that can be preempted or interrupt itself so it can continue running at a later time.
    // Each thread self-manages his stack, since this is also used during preempting
    class Thread {
    public:
        Thread(void(*entry)());
        Thread(Thread& thread);
        Thread(Thread&& thread);
        ~Thread();

        Thread& operator=(Thread& t);
        Thread& operator=(Thread&& t);

        void enter();

    private:
        char* _stack;
        
        // The esp of the running thread
        u32 esp;

        // If the thread has finished or not
        bool _finished;
    };

    // Exits the current thread and saves it state, so it can be resumed at a later date.
    void exit();
}

#endif