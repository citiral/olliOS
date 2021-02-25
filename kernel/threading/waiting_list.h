#ifndef __WAITING_LIST_H
#define __WAITING_LIST_H

#include "spinlock.h"
#include "thread.h"

namespace threading {
class WaitingList {
public:
    WaitingList();
    ~WaitingList();

    void add_blocked_thread(Thread* thread);
    void unblock_next_thread();
    void unblock_all_threads();

    //static void add_to_multiple(Thread* thread, WaitingList** list, size_t count);

private:
    Thread* _waiting;
    Spinlock _lock;
};
}

#endif