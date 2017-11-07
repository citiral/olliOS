#include "sleep.h"
#include "apic.h"
#include "interrupt.h"
#include "cdefs.h"


bool sleepIRQ(u32 irq, void* stack, void* data) {
    *(bool*)data = true;
    return true;
}

void apicSleep(uint32_t millisecond) {
    volatile bool woken = false;
    interrupts.registerIRQ(0xFC, sleepIRQ, (void*)&woken);
    apic::setSleep((apic::busFrequency/16000) * millisecond , true);
    while (!woken) {
        __asm__ __volatile ("HLT");
    }
    interrupts.unregisterIRQ(0xFC, sleepIRQ, (void*)&woken);
}

void sleep(uint32_t millisecond) {
    if (apic::enabled()) {
        apicSleep(millisecond);
    } else {
        LOG_ERROR("No sleeping mechanism found.");
    }
}