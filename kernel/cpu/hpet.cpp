#include "hpet.h"
#include "acpi.h"
#include "stdio.h"
#include "types.h"
#include "interrupt.h"
#include "../memory/virtual.h"

using namespace hpet;

enum class HPETRegisterIndex: size_t {
    GeneralCapabilities = 0x000,
    GeneralConfiguration = 0x010 / 8,
    GeneralInterruptStatus = 0x020 / 8,
    CounterValue = 0x0F0 / 8,
    TimerConfigurationAndCapabilities = 0x100 / 8,
    TimerComparator = 0x108 / 8,
    TimerFSBInterruptRoute = 0x110 / 8,
};

Hpet hpet::hpet;

extern "C" void intHandlerHpet_asm();
extern "C" void intHandlerHpet(uint32_t interrupt) {
    hpet::hpet.process();
    end_interrupt(interrupt);
}


bool Hpet::init()
{
    queuedCallbacks = nullptr;

    // If no HPET acpi header is present, it means there is no HPET on this system.
    acpiHeader = (acpi::HPETHeader*)acpi::findHeader("HPET");
    if (acpiHeader == nullptr) {
        return false;
    } else {
        acpi::mapHeader((acpi::ACPISDTHeader*) acpiHeader);
    }

    // Map the hpet registers to virtual memory so we can access them
    hpetRegisters = (volatile uint64_t*) memory::kernelPageDirectory.bindPhysicalPage((void*) acpiHeader->address.address, memory::UserMode::Supervisor);
    if (hpetRegisters == nullptr) {
        return false;
    }

    // Calculate the hpet frequency in microseconds. We don't need more exact timing.
    auto genCap = readGeneralCapabilities();
    frequency = 1000000000l / genCap.clockPeriod;

    // Make sure all timer interrupts are disabled.
    // If we want them, we will enable them later.
    for (int i = 0 ; i < genCap.timerCount + 1 ; i++) {
        auto timConf = readTimerConfiguration(i);
        timConf.interruptsEnabled = 0;
        writeTimerConfiguration(i, timConf);
    }

    // Enable the first timer
    auto timer1 = readTimerConfiguration(0);
    if ((timer1.interruptRoutingCapability & 0x4) == 0) {
        printf("Error, hpet timer does not support IRQ2");
        return false;
    }
    timer1.interruptsEnabled = 1;
    timer1.interruptRoute = 2;
    timer1.periodic = 0;
    timer1.levelTriggered = 0;
    timer1.force32Bit = 0;
    writeTimerConfiguration(0, timer1);
    writeTimerComparator(0, 0);

    timer1.interruptsEnabled = 1;
    writeTimerConfiguration(0, timer1);

    // Enable the interrupt handler we will use for all hpet interrupts
    idt.getEntry(INT_PREEMPT).setOffset((u32)intHandlerHpet_asm);

    // Enable the counter, from now on it will start counting
    auto genConf = readGeneralConfiguration();
    genConf.enabled = 1;
    genConf.legacyReplacementEnabled = 0;
    writeGeneralConfiguration(genConf);

    return true;
}

void Hpet::process()
{
    uint64_t counter = readMainCounter();
    
    bool b = CLI();
    while (queuedCallbacks != nullptr && queuedCallbacks->accumulator <= counter) {
        auto temp = queuedCallbacks;
        auto cb = temp->cb;
        auto ctx = temp->ctx;
        auto next = temp->next;
        queuedCallbacks = next;
        STI(b);
        delete temp;
        cb(ctx);
        b = CLI();
    }
    STI(b);

    b = CLI();
    if (queuedCallbacks != nullptr) {
        writeTimerComparator(0, queuedCallbacks->accumulator);
    }
    STI(b);
}

uint64_t Hpet::get_elapsed_us()
{
    return readMainCounter() / frequency;
}

void Hpet::wait(uint64_t us, HpetCallback cb, void* ctx)
{
    uint64_t timestamp = (get_elapsed_us() + us) * frequency;
    HpetQueuedCallback* hqc = new HpetQueuedCallback;
    hqc->ctx = ctx;
    hqc->cb = cb;
    hqc->accumulator = timestamp;
    hqc->next = nullptr;

    bool updateCounter = true;

    bool f = CLI();
    if (queuedCallbacks == nullptr) {
        queuedCallbacks = hqc;
    } else {
        HpetQueuedCallback* prev = nullptr;
        HpetQueuedCallback* current = queuedCallbacks;

        while (1) {
            if (current == nullptr) {
                prev->next = hqc;
                updateCounter = false;
                break;
            }
            else if (current->accumulator >= timestamp) {
                hqc->next = current;
                if (prev == nullptr) {
                    queuedCallbacks = hqc;
                } else {
                    prev->next = hqc;
                }
                break;
            } else {
                prev = current;
                current = current->next;
                updateCounter = false;
            }
        }
    }
    STI(f);

    if (updateCounter) {
        writeTimerComparator(0, timestamp);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
Hpet::GeneralCapabilities Hpet::readGeneralCapabilities()
{
    uint64_t val = hpetRegisters[(size_t)HPETRegisterIndex::GeneralCapabilities];
    return *reinterpret_cast<GeneralCapabilities*>(&val);
}

void Hpet::writeGeneralCapabilities(GeneralCapabilities val)
{
    uint64_t v = *reinterpret_cast<uint64_t*>(&val);
    hpetRegisters[(size_t)HPETRegisterIndex::GeneralCapabilities] = v;
}

Hpet::GeneralConfiguration Hpet::readGeneralConfiguration()
{
    uint64_t val = hpetRegisters[(size_t)HPETRegisterIndex::GeneralConfiguration];
    return *reinterpret_cast<GeneralConfiguration*>(&val);
}

void Hpet::writeGeneralConfiguration(GeneralConfiguration val)
{
    uint64_t v = *reinterpret_cast<uint64_t*>(&val);
    hpetRegisters[(size_t)HPETRegisterIndex::GeneralConfiguration] = v;
}

Hpet::GeneralInterruptStatus Hpet::readGeneralInterruptStatus()
{
    uint64_t val = hpetRegisters[(size_t)HPETRegisterIndex::GeneralInterruptStatus];
    return *reinterpret_cast<GeneralInterruptStatus*>(&val);
}

void Hpet::writeGeneralInterruptStatus(GeneralInterruptStatus val)
{
    uint64_t v = *reinterpret_cast<uint64_t*>(&val);
    hpetRegisters[(size_t)HPETRegisterIndex::GeneralInterruptStatus] = v;
}

uint64_t Hpet::readMainCounter()
{
    // because the read is performed as 2 32-bit reads on x86, simply reading the countervalue is not safe.
    // if the lower 32-bits of the register were to roll over into the upper 32-bits of the register, the concatenated reads will be wrong.
    // a safety mechanism is implemented that checks no rollover happened during the read

    do {
        uint64_t val1 = hpetRegisters[(size_t)HPETRegisterIndex::CounterValue];
        uint64_t val2 = hpetRegisters[(size_t)HPETRegisterIndex::CounterValue];
        
        uint32_t top1 = val1 >> 32;
        uint32_t top2 = val2 >> 32;

        if (top1 == top2) {
            return val2;
        }
    } while (1);
}

void Hpet::writeMainCounter(uint64_t val)
{
    hpetRegisters[(size_t)HPETRegisterIndex::CounterValue] = val;
}

Hpet::TimerConfiguration Hpet::readTimerConfiguration(uint32_t timer)
{
    uint64_t val = hpetRegisters[(size_t)HPETRegisterIndex::TimerConfigurationAndCapabilities + 0x20/8*timer];
    return *reinterpret_cast<TimerConfiguration*>(&val);
}

void Hpet::writeTimerConfiguration(uint32_t timer, TimerConfiguration val)
{
    uint64_t v = *reinterpret_cast<uint64_t*>(&val);
    hpetRegisters[(size_t)HPETRegisterIndex::TimerConfigurationAndCapabilities + 0x20/8*timer] = v;
}

uint64_t Hpet::readTimerComparator(uint32_t timer)
{
    uint64_t val = hpetRegisters[(size_t)HPETRegisterIndex::TimerComparator + 0x20/8*timer];
    return val;
}

void Hpet::writeTimerComparator(uint32_t timer, uint64_t val)
{
    hpetRegisters[(size_t)HPETRegisterIndex::TimerComparator + 0x20/8*timer] = val;
}
#pragma GCC diagnostic pop


void Hpet::printInfo()
{
    printf("hardware_rev_id: %u\n", acpiHeader->hardware_rev_id);
    printf("comparator_count: %u\n", acpiHeader->comparator_count);
    printf("counter_size: %u\n", acpiHeader->counter_size);
    printf("reserved: %u\n", acpiHeader->reserved);
    printf("legacy_replacement: %u\n", acpiHeader->legacy_replacement);
    printf("pci_vendor_id: %x\n", acpiHeader->pci_vendor_id);
    printf("hpet_number: %u\n", acpiHeader->hpet_number);
    printf("minimum_tick: %u\n", acpiHeader->minimum_tick);
    printf("page_protection: %u\n", acpiHeader->page_protection);
    printf("address_space_id: %X\n", acpiHeader->address.address_space_id);
    printf("register_bit_width: %X\n", acpiHeader->address.register_bit_width);
    printf("register_bit_offset: %X\n", acpiHeader->address.register_bit_offset);
    printf("reserved: %X\n", acpiHeader->address.reserved);
    printf("address: %X\n", acpiHeader->address.address);

    auto genCap = readGeneralCapabilities();
    printf("revisionId: %lx\n", genCap.revision);
    printf("timerAmount: %lx\n", genCap.timerCount);
    printf("64bit: %lx\n", genCap.is64BitCapable);
    printf("legacy supported: %lx\n", genCap.isLegacyReplacementCapable);
    printf("vendorid: %lx\n", genCap.vendorId);
    printf("counterPeriod: %lx\n", genCap.clockPeriod);
    printf("Frequency: %lx\n", frequency);

    for (int i = 0 ; i < genCap.timerCount + 1 ; i++) {
        auto timConf = readTimerConfiguration(i);
        printf("timer %d:", i);
        printf(" 64bit=%lu", timConf.is64Bit);
        printf(" fsb=%lu", timConf.FSBInterruptMappingSupported);
        printf(" periodic=%lx", timConf.periodicCapable);
        printf(" routing=%x", timConf.interruptRoutingCapability);
        printf(" interrupt=%lx\n", timConf.interruptsEnabled);
    }
}