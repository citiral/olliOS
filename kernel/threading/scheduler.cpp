#include "scheduler.h"
#include "cpu/apic.h"
#include "cdefs.h"
#include "process.h"

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

        _threads.push(thread);
        _lock.release();
        STI(eflag);
    } else {
        eflag = CLI();
        _lock.lock();
        _threads.push(thread);
        _lock.release();
        STI(eflag);
    }
}

bool Scheduler::enter() {
    _lock.lock();
    bool eflag = CLI();

    // if there are no more threads to execute, do nothing
    // TODO wait process or something
    if (_threads.is_empty()) {
        _lock.release();
        STI(eflag);
        return false;
    }

    // otherwise get the next thread to execute
    Thread* thread = _threads.pop();

    // run the thread
    _lock.release();
    STI(eflag);
    if (!thread->enter()) {
        // if the thread is not finished, schedule it again, except if it is blocking on some resource
        if (!thread->blocking()) {
            schedule(thread);
        }
    } else {
        if (thread->process != nullptr) {
            if (thread->process->state == ProcessState::PendingDestruction) {
                delete thread->process;
            } else {
                thread->process->set_state(ProcessState::Stopped);
            }
        } else {
            delete thread;
        }
    }

    return true;
}
