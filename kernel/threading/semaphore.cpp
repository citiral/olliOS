#include "threading/semaphore.h"
#include "threading/thread.h"
#include "cdefs.h"

using namespace threading;

Semaphore::Semaphore(int count): _lock(), _count(count) {

}

void Semaphore::lock() {
    // We keep trying to lock until we get it
    while (true) {
        CLI();
        // we make sure this a critical section using a spinlock
        _lock.lock();

        // if the current value is bigger than 0, the lock is successful and we can continue
        if (_count > 0) {
            _count--;
            _lock.release();
            STI();
            return;
        }// if the value is zero, we can schedule ourselves and try again at a later time
        else {
            _lock.release();
            STI();
            threading::exit();
        }
    }
}

bool Semaphore::try_lock() {
    CLI();
    // we make sure this a critical section using a spinlock
    _lock.lock();

    // if the current value is bigger than 0, the lock is successful and we can continue
    if (_count > 0) {
        _count--;
        _lock.release();
        STI();
        return true;
    }
    else {
        _lock.release();
        STI();
        return false;
    }
}

void Semaphore::release() {
    CLI();
    _lock.lock();
    _count++;
    _lock.release();
    STI();
}