#ifndef __THREAD_H_
#define __THREAD_H_

#include <types.h>
#include "cdefs.h"
#include "kstd/utility.h"
#include "util/unique.h"
#include <string.h>

extern "C" void __attribute__ ((noinline)) thread_interrupt();
extern "C" u32* __attribute__ ((noinline)) get_parent_stack();
extern "C" bool __attribute__ ((noinline)) is_current_core_in_thread();
extern "C" u32 __attribute__ ((noinline)) thread_enter(volatile u32* pEsp, volatile u32* esp);
extern "C" void __attribute__ ((noinline)) thread_exit(volatile u32* pEsp);
extern "C" void __attribute__ ((noinline)) thread_finished();
extern "C" void __attribute__ ((noinline)) thread_entry();

template<u32 SIZE, class ... ARGS>
void threadingFunctionWrapper(void(*func)(ARGS...), u32 size, ARGS ... args) {
    func(args...);
}

namespace threading {
    
    extern UniqueGenerator<u32> pidGenerator;

    // We define a thread's stack size to be 16kb, which should be more than enough.
    // TODO some thread protected mechanisms, through paging perhaps?
    #define THREAD_STACK_SIZE 0x1000*16

    // A thread manages a single running function, that can be preempted or interrupt itself so it can continue running at a later time.
    // Each thread self-manages his stack, since this is also used during preempting
    class Thread {
    public:
        // initializes a thread were the called function gets passed the given arguments
        template<class ... ARGS>
        Thread(void(*entry)(ARGS...), ARGS ... args): _finished(false), _id(pidGenerator.next()), _blocking(false) {
            // A new thread allocates his own stack
            _stack = new char[THREAD_STACK_SIZE];
            memset(_stack, 0, THREAD_STACK_SIZE);

            // we want the arguments to be placed below the entry address, so starting from esp-8 to esp-X
            u32 argStackSize = stackSizeOfArguments(args...);
            initializeArguments(argStackSize + 4, args...);
            
            // we first place the asm routine the thread will return to when finished
            *(u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 8) = argStackSize;

            // then we place the entry address
            *(u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 12) = (u32)entry;
            *(u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 16) = (u32)thread_entry;

            // then 36 bytes zero (pushad + pushfd) but those are already zero with memset
            esp = (u32)(_stack + THREAD_STACK_SIZE - argStackSize - 52);
        }

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
        void initializeArguments(u32) {
            
        }

        template<class ARG, class... ARGS>
        void initializeArguments(u32 offset, ARG arg, ARGS ... args) {
            // first we place the current argument at the stack on the given offset
            ARG* target = (ARG*)(_stack + THREAD_STACK_SIZE - offset);          
            *target = arg;  

            // then we place the remaining arguments, and return the final offset
            initializeArguments(offset  - stackSizeOfArguments(arg), args...);
        }

        constexpr u32 stackSizeOfArguments() {
            return 0;
        }

        template<class ARG, class... ARGS>
        constexpr u32 stackSizeOfArguments(ARG, ARGS ... args) {
            return std::roundup(sizeof(ARG), 4ul) + stackSizeOfArguments(args...);
        }


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