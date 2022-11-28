#ifndef __HPET_H_
#define __HPET_H_

#include "acpi.h"
#include "types.h"
#include <stdint.h>

namespace hpet {
    typedef void (*HpetCallback)(void* context);

    struct HpetQueuedCallback {
        HpetCallback cb;
        void* ctx;
        uint64_t accumulator;
        HpetQueuedCallback* next;
    };

    class Hpet {
    public:
        bool init();

        void process();

        uint64_t get_elapsed_us();
        void wait(uint64_t us, HpetCallback cb, void* ctx);

    private:
        struct GeneralCapabilities {
            uint64_t revision:8;
            uint64_t timerCount:5;
            uint64_t is64BitCapable:1;
            uint64_t _reserved:1;
            uint64_t isLegacyReplacementCapable:1;
            uint64_t vendorId:16;
            uint64_t clockPeriod:32;
        } __attribute__((packed));

        struct GeneralConfiguration {
            uint64_t enabled:1;
            uint64_t legacyReplacementEnabled:1;
            uint64_t _reserved:62;
        } __attribute__((packed));

        struct GeneralInterruptStatus {
            uint32_t _reserved;
            uint32_t interruptTriggered;
        };

        struct TimerConfiguration {
            uint64_t _reserved1:1;
            uint64_t levelTriggered:1;
            uint64_t interruptsEnabled:1;
            uint64_t periodic:1;
            uint64_t periodicCapable:1;
            uint64_t is64Bit:1;
            uint64_t writeEnableAccumulator:1;
            uint64_t _reserved2:1;
            uint64_t force32Bit:1;
            uint64_t interruptRoute:5;
            uint64_t FSBInterruptMappingEnabled:1;
            uint64_t FSBInterruptMappingSupported:1;
            uint64_t _reserved3:16;
            uint64_t interruptRoutingCapability:32;
        };

        GeneralCapabilities readGeneralCapabilities();
        void writeGeneralCapabilities(GeneralCapabilities val);
        GeneralConfiguration readGeneralConfiguration();
        void writeGeneralConfiguration(GeneralConfiguration val);
        GeneralInterruptStatus readGeneralInterruptStatus();
        void writeGeneralInterruptStatus(GeneralInterruptStatus val);
        uint64_t readMainCounter();
        void writeMainCounter(uint64_t val);

        TimerConfiguration readTimerConfiguration(uint32_t timer);
        void writeTimerConfiguration(uint32_t timer, TimerConfiguration val);
        uint64_t readTimerComparator(uint32_t timer);
        void writeTimerComparator(uint32_t timer, uint64_t val);

        void printInfo();

        acpi::HPETHeader* acpiHeader;
        volatile uint64_t* hpetRegisters;
        uint64_t frequency; // in microseconds
        HpetQueuedCallback* queuedCallbacks;
    };

    extern Hpet hpet;
}

#endif