#ifndef __SEMAPHORE_H_
#define __SEMAPHORE_H_

#include "threading/spinlock.h"
#include <types.h>

// A semaphore can only be executed inside the context of a thread, since it exits the current thread when having to halt.
namespace threading {
    class Semaphore {
    public:
        Semaphore(int count);

        void lock();
        bool try_lock();
        void release();

    private:
        Spinlock _lock;
        u32 _count;
    };
}


#endif