#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#include "threading/spinlock.h"
#include "threading/thread.h"
#include "kstd/linkedlist.h"
#include "kstd/shared_ptr.h"

namespace threading {
    class Scheduler {
    public:
        Scheduler();
        
        // schedule a thread to be run at a later time
        void schedule(Thread* thread);

        // Entry point for a cpu to start running scheduled threads
        // returns false if no thread has been entered
        bool enter();
    private:
        // TODO make this a double linked list
        LinkedList<Thread*> _threads;

        // a spinlock to make member functions atomic
        Spinlock _lock;
    };

    extern Scheduler* scheduler;    
}

#endif