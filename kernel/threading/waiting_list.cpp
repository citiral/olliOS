#include "waiting_list.h"
#include "scheduler.h"
#include "thread.h"

using namespace threading;

WaitingList::WaitingList(): _waiting(nullptr), _lock()
{

}

WaitingList::~WaitingList()
{

}

void WaitingList::add_blocked_thread(Thread* thread)
{
    bool eflag = CLI();
    _lock.lock();

    if (!_waiting) {
        _waiting = thread;
    } else {
        Thread* last = _waiting;
        while (last->nextWaiting) {
            last = last->nextWaiting;
        }
        last->nextWaiting = thread;
    }
    thread->setBlocking(true);
    _lock.release();
    STI(eflag);
}

void WaitingList::unblock_next_thread()
{
    bool eflag = CLI();
    _lock.lock();
    Thread* toSchedule = nullptr;

    // Skip until we succcessfully unblocked a thread ourselves
    while (_waiting && (_waiting->setBlocking(false) == false)) {
        _waiting = _waiting->nextWaiting;
    }

    // Unblock the waiting thread if there is one
    if (_waiting) {
        toSchedule = _waiting;
        _waiting = _waiting->nextWaiting;
        toSchedule->nextWaiting = nullptr;
    }

    _lock.release();
    STI(eflag);

    if (toSchedule) {
        threading::scheduler->schedule(toSchedule);
    }
}


void WaitingList::unblock_all_threads()
{
    Thread* toSchedule;

    do {
        bool eflag = CLI();
        _lock.lock();
    
        toSchedule = nullptr;

        if (_waiting) {
            toSchedule = _waiting;
            _waiting->setBlocking(false);
            _waiting = _waiting->nextWaiting;
            toSchedule->nextWaiting = nullptr;
        }

        _lock.release();
        STI(eflag);

        if (toSchedule) {
            threading::scheduler->schedule(toSchedule);
        }
    } while (toSchedule != nullptr);
}

/*void WaitingList::add_to_multiple(threading::Thread* thread, WaitingList** list, size_t count)
{
    bool eflag = CLI();
    thread->setBlocking(true);

    for (size_t i = 0 ; i < count ; i++)
    {
        list[i]->_lock.lock();

        if (!list[i]->_waiting) {
            list[i]->_waiting = thread;
        } else {
            Thread* last = list[i]->_waiting;
            while (last->nextWaiting) {
                last = last->nextWaiting;
            }
            last->nextWaiting = thread;
        }
        list[i]->_lock.release();
    }

    STI(eflag);
}*/