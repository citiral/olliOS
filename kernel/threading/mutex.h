#ifndef __MUTEX_H_
#define __MUTEX_H_

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
    };
}


#endif