#ifndef __THREAD_H_
#define __THREAD_H_

#include <types.h>
#include "cdefs.h"
#include "memory/virtual.h"
#include "kstd/shared_ptr.h"
#include "kstd/utility.h"
#include "util/unique.h"
#include <string.h>
#include "process.h"

extern "C" void __attribute__ ((noinline)) thread_interrupt();
extern "C" bool __attribute__ ((noinline)) is_current_core_in_thread();
extern "C" u32 __attribute__ ((noinline)) thread_enter(volatile u32* pEsp, volatile u32* esp);
extern "C" void __attribute__ ((noinline)) thread_exit(volatile u32* pEsp);
extern "C" void __attribute__ ((noinline)) thread_finished();
extern "C" void __attribute__ ((noinline)) thread_entry();

template<class T, class ... ARGS>
void threadingFunctionWrapper(void(T::*func)(ARGS...), T* c, ARGS ... args) {
    (c->*func)(args...);
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
        Thread(Process* process, volatile char* stack, void(*entry)(ARGS...), ARGS ... args): process(process), _stack(stack), _finished(false), _id(pidGenerator.next()), _blocking(false) {
            // A new thread allocates his own stack, if none is supplied
            _ownsStack = _stack == nullptr;
            if (_stack == nullptr) {
                _stack = new char[THREAD_STACK_SIZE];
            }
            memset((void*) _stack, 0, THREAD_STACK_SIZE);

            // we want the arguments to be placed below the entry address, so starting from esp-8 to esp-X
            u32 argStackSize = stackSizeOfArguments(args...);
            initializeArguments(argStackSize + 4, args...);

            // we first place the asm routine the thread will return to when finished
            *(volatile u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 8) = argStackSize;

            // then we place the entry address
            *(volatile u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 12) = (u32)entry;
            *(volatile u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 16) = (u32)thread_entry;

            // then 36 bytes zero (pushad + pushfd) but those are already zero with memset
            esp = (u32)(_stack + THREAD_STACK_SIZE - argStackSize - 52);
        }

        // initializes a thread where the called function gets passed the given arguments
        template<class T, class ... ARGS>
        Thread(Process* process, volatile char* stack, void(T::*entry)(ARGS...), T* c, ARGS ... args): process(process), _stack(stack),  _finished(false), _id(pidGenerator.next()), _blocking(false) {
            // A new thread allocates his own stack
            _ownsStack = _stack == nullptr;
            if (_stack == nullptr) {
                _stack = new char[THREAD_STACK_SIZE];
                memset((void*) _stack, 0, THREAD_STACK_SIZE);
            }

            // we want the arguments to be placed below the entry address, so starting from esp-8 to esp-X
            u32 argStackSize = stackSizeOfArguments(entry, c, args...);
            initializeArguments(argStackSize + 4, entry, c, args...);

            // we first place the asm routine the thread will return to when finished
            *(volatile u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 8) = argStackSize;

            // then we place the entry address
            *(volatile u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 12) = (u32)threadingFunctionWrapper<T, ARGS...>;
            *(volatile u32*)(_stack + THREAD_STACK_SIZE - argStackSize - 16) = (u32)thread_entry;

            // then 36 bytes zero (pushad + pushfd) but those are already zero with memset
            esp = (u32)(_stack + THREAD_STACK_SIZE - argStackSize - 52);
        }

        Thread() {}

        Thread(Thread& thread) = delete;
        Thread(Thread&& thread) = delete;
        ~Thread();

        Thread& operator=(Thread& t) = delete;
        Thread& operator=(Thread&& t) = delete;

        // Clones the thread.
        // Note: This function is not safe to execute while the given thread is running
        Thread* clone();

        bool enter();

        u32 id();

        bool finished();

        bool blocking();
        void setBlocking(bool blocking);

        // Kills the thread by setting finished to true. If the thread is still running, it will only be shut down next time it is scheduled.
        void kill();

        // Optional process of the thread
        Process* process;
        
    private:
        void initializeArguments(u32) {
            
        }

        template<class ARG, class... ARGS>
        void initializeArguments(u32 offset, ARG arg, ARGS ... args) {
            // first we place the current argument at the stack on the given offset
            //ARG* target = (ARG*)();
            new ((void*) _stack + THREAD_STACK_SIZE - offset) ARG(arg);
            //*target = arg;

            // then we place the remaining arguments, and return the final offset
            initializeArguments(offset  - stackSizeOfArguments(arg), args...);
        }

        constexpr u32 stackSizeOfArguments() {
            return 0;
        }

        template<class ARG>
        constexpr u32 stackSizeOfArguments(ARG) {
            return std::roundup(sizeof(ARG),  alignof(ARG));
        }

        template<class ARG, class... ARGS>
        constexpr u32 stackSizeOfArguments(ARG, ARGS ... args) {
            return std::roundup(sizeof(ARG),  alignof(ARG)) + stackSizeOfArguments(args...);
        }


        // The stack used by the thread
        volatile char* _stack;

        // True if the thread should deallocate the stack itself
        bool _ownsStack;
        
        // The esp of the running thread
        volatile u32 esp;

        // If the thread has finished or not
        volatile bool _finished;

        // The ID of the thread
        u32 _id;

        // Whether or not the current thread is blocking, by for example waiting on a semphore or a mutex
        bool _blocking;
    };

    // Forks the current thread.
    // Returns the child thread on the parent.
    // Returns NULL for the child
    Thread* fork();

    // Exits the current thread and saves it state, so it can be resumed at a later date.
    void exit();

    // Returns true if the given physical core is currently running a thread
    bool is_core_in_thread(u8 core);

    // Returns true if the calling physical core is currently running a thread
    bool is_current_core_in_thread();

    Thread* currentThread();
}

#endif