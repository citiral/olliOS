#include "threading/mutex.h"
#include "threading/scheduler.h"
#include "cdefs.h"

using namespace threading;

Mutex::Mutex(): _locked(0), _waitingList() {

}

void Mutex::lock() {
    // Keep trying to exchange the value with 1 when the value was 0 before
    volatile register u8 v = 1;
    
    while (true) {
        // exchange the current value with v. if the mutex was free, v is now 0
        __asm__ volatile ("xchg %0, %1; pause " : "+m"(_locked), "=a" (v) : "1" (v));
        
        if (v == 0) {
            return;
        }

        _waitingList.add_blocked_thread(threading::currentThread());

        threading::exit();
    }
}

bool Mutex::try_lock() {
    volatile register u8 v = 1;
    
    // exchange the current value with v. if the mutex was free, v is now 0
    __asm__ volatile ("xchg %0, %1; pause " : "+m"(_locked), "=a" (v) : "1" (v));

    return (v == 0);
}

void Mutex::release() {
    // Exchange the current value with 0
    volatile register u8 v = 0;
    __asm__ volatile ("xchg %0, %1" : "+m"(_locked), "=a" (v) : "1" (v));

    _waitingList.unblock_next_thread();
}