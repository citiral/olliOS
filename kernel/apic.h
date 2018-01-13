#ifndef __OLLIOS_APIC_H
#define __OLLIOS_APIC_H

#include "types.h"
#include "stdint.h"

#define APIC_EOI_REGISTER 44
#define APIC_SIV_REGISTER 60
#define APIC_INT_COMMAND1_REGISTER 192
#define APIC_INT_COMMAND2_REGISTER 196
#define APIC_LAPIC_TIMER_REGISTER 200
#define APIC_LINT0_REGISTER 212
#define APIC_LINT1_REGISTER 216
#define APIC_ERROR_REGISTER 160
#define APIC_TIMER_INITIAL_COUNT_REGISTER 224
#define APIC_TIMER_CURRENT_COUNT_REGISTER 228
#define APIC_TIMER_DIVIDE_REGISTER 248
#define APIC_IO_SEL 0
#define APIC_IO_WIN 4
#define APIC_IO_ID_OFFSET 0x0
#define APIC_IO_VER_OFFSET 0x1
#define APIC_IO_ARBITRATION_OFFSET 0x2
#define APIC_IO_REDIRECTION0_OFFSET 0x10
#define APIC_IO_REDIRECTION1_OFFSET 0x11

namespace apic {

    // The frequency of the external bus. This is used for the apic timer, but is exposed for everyone to use.
    extern uint64_t busFrequency;

    // Returns true if the apic is enabled (and used). If this is false, the PIC is used instead
    bool enabled();

    // Ends an interrupt. The given interrupt doesnt't really matter.
    void endInterrupt(u32 interrupt);

    // Starts the sleep timer with the given state
    void setSleep(uint32_t count, bool onetime);

    // initializes the apic
    void Init();

    // Starts all cpus
    void StartAllCpus(void* startAddress);
}


#endif /* end of include guard: __PIC_H */
