#ifndef __SPINLOCK_H_
#define __SPINLOCK_H_

#include <types.h>

namespace threading {
    class Spinlock {
    public:
        Spinlock();

        void lock();
        bool try_lock();
        void release();

    private:
        u8 _value;
    };
}

#endif