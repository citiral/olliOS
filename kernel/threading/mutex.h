#ifndef __MUTEX_H_
#define __MUTEX_H_

#include "threading/spinlock.h"
#include "threading/thread.h"
#include "threading/waiting_list.h"
#include <types.h>

namespace threading {
    class Mutex {
    public:
        Mutex();

        void lock();
        bool try_lock();
        void release();

    private:
        int _locked;
        WaitingList _waitingList;
    };
}


#endif