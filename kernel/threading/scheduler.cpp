#include "scheduler.h"
#include "cpu/apic.h"
#include "cdefs.h"

using namespace threading;

Scheduler* threading::scheduler = nullptr;

Scheduler::Scheduler(): _threads(), _lock() {
}

void Scheduler::schedule(Thread* thread) {
    bool eflag;
    // If a thread gets interrupted while it has the scheduler lock, everything breaks.
    // For this reason we make it a critical section that something should never be interrupted in
    if (threading::is_core_in_thread(apic::id())) {
        while (true) {
            eflag = CLI();
            if (_lock.try_lock()) {
                break;
            }
            STI(eflag);
            threading::exit();
        }

        _threads.push_back(thread);
        _lock.release();
        STI(eflag);
    } else {
        _lock.lock();
        _threads.push_back(thread);
        _lock.release();
    }
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
    if (!thread->enter()) {
        // if the thread is not finished, schedule it again
        schedule(thread);
    } else {
        if (thread->process() != nullptr) {
            // Watch out, the moment this is set to Stopped the thread can get deleted
            thread->process()->state = ProcessState::Stopped;
        }
    }
}
