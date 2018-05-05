#ifndef __THREAD_H_
#define __THREAD_H_

#include <types.h>
#include "cdefs.h"

extern "C" void __attribute__ ((noinline)) thread_interrupt();
extern "C" u32* __attribute__ ((noinline)) get_parent_stack();
extern "C" bool __attribute__ ((noinline)) is_current_core_in_thread();

namespace threading {
    // We define a thread's stack size to be 16kb, which should be more than enough.
    // TODO some thread protected mechanisms, through paging perhaps?
    #define THREAD_STACK_SIZE 0x1000*16

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

        u32 pid();

        bool finished();

        bool blocking();
        void setBlocking(bool blocking);

    private:
        char* _stack;
        
        // The esp of the running thread
        volatile u32 esp;

        // If the thread has finished or not
        volatile bool _finished;

        // The ID of the thread
        u32 _id;

        // Whether or not the current thread is blocking, by for example waiting on a semphore or a mutex
        bool _blocking;
    };

    // Exits the current thread and saves it state, so it can be resumed at a later date.
    void exit();

    // Returns true if the given physical core is currently running a thread
    bool is_core_in_thread(u8 core);

    // Returns true if the calling physical core is currently running a thread
    bool is_current_core_in_thread();
}

#endif