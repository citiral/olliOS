#include "spinlock.h"

using namespace threading;

Spinlock::Spinlock(): _value(0) {

}

void Spinlock::lock() {
    // Keep trying to exchange the value with 1 when the value was 0 before
    volatile register u8 v = 1;
    do {
        // exchange the current value with v. if the spinlock was free, v is now 0
        __asm__ ("xchg %0, %1; pause " : "+m"(_value), "=a" (v) : "1" (v));
    } while (v == 1);
}

bool Spinlock::try_lock() {
    volatile register u8 v = 0;
    // exchange the current value with v. if the spinlock was free, v is now 0
    __asm__ ("xchg %0, %1; pause " : "+m"(_value), "=a" (v) : "1" (v));

    return (v == 0);
}

void Spinlock::release() {
    // Exchange the current value with 0
    volatile register u8 v = 0;
    __asm__ ("xchg %0, %1" : "+m"(_value), "=a" (v) : "1" (v));
}