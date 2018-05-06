#include "sleep.h"
#include "apic.h"
#include "interrupt.h"
#include "cdefs.h"


bool sleepIRQ(u32 irq, void* stack, void* data) {
    UNUSED(irq);
    UNUSED(stack);
    *(bool*)data = true;
    return true;
}

void apicSleep(uint32_t millisecond) {
    UNUSED(millisecond);
    LOG_UNIMPLEMENTED();
    return;
}

void apicHardSleep(uint32_t millisecond) {
    apic::registers[APIC_TIMER_CURRENT_COUNT_REGISTER] = 0xFFFFFFFFu;
    uint32_t target = (apic::busFrequency * millisecond) / 1000;
    uint32_t current;
    do {
        __asm__ __volatile ("HLT");
        current = apic::registers[APIC_TIMER_CURRENT_COUNT_REGISTER];
    } while (0xFFFFFFFFu - current < target);
}

void sleep(uint32_t millisecond) {
    if (apic::enabled()) {
        apicSleep(millisecond);
    } else {
        LOG_ERROR("No sleeping mechanism found.");
    }
}

void hardSleep(uint32_t millisecond) {
    if (apic::enabled()) {
        apicHardSleep(millisecond);
    } else {
        LOG_ERROR("No sleeping mechanism found.");
    }
}