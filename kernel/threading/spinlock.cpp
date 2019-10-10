#include "spinlock.h"

using namespace threading;

Spinlock::Spinlock(): _value(0) {

}

void Spinlock::lock() {
    // Keep trying to exchange the value with 1 when the value was 0 before
    while (!__sync_bool_compare_and_swap(&_value, 0, 1));
    __sync_synchronize();
}

bool Spinlock::try_lock() {
    // exchange the current value with v. if the spinlock was free, v is now 0
    return __sync_bool_compare_and_swap(&_value, 0, 1);
}

void Spinlock::release() {
    // Exchange the current value with 0
    __sync_synchronize();
    _value = 0;
}
