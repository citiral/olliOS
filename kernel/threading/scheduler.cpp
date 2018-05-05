#include "scheduler.h"
#include "cdefs.h"

using namespace threading;

Scheduler* threading::scheduler = nullptr;

Scheduler::Scheduler(): _lock(), _threads() {
}

void Scheduler::schedule(Thread* thread) {
    _lock.lock();
    _threads.push_back(thread);
    _lock.release();
}

void Scheduler::enter() {
    _lock.lock();

    // if there are no more threads to execute, do nothing
    // TODO wait process or something
    if (_threads.size() == 0) {
        _lock.release();
        return;
    }

    // otherwise get the next thread to execute
    Thread* thread = _threads[0];
    _threads.erase(0);

    // run the thread
    _lock.release();
    thread->enter();

    // if the thread is not finished, schedule it again
    if (!thread->finished()) {
        schedule(thread);
    }
}