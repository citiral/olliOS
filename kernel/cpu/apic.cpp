#include "acpi.h"
#include "linker.h"
#include "apic.h"
#include "gdt.h"
#include "interrupt.h"
#include "memory/virtual.h"
#include "memory/physical.h"
#include "pic.h"
#include "cpuid.h"
#include "io.h"
#include "kstd/vector.h"
#include "sleep.h"
#include "cdefs.h"
#include <stdio.h>

// Functions and constant defined by the SMP trampoline
extern "C" void smp_trampoline_entry();
extern "C" void end_smp_trampoline_entry();
extern u16 smp_gdt_size;
extern u32 smp_gdt_offset;
extern void* smp_stack;
extern memory::PageDirectory* smp_page;

using namespace acpi;

namespace apic {
    // A pointer to the registers of the APIC. These are by default mapped to physical page FEE00xxx
    uint32_t volatile* registers;
    std::vector<MADTIoEntry*> ioApics;
    std::vector<MADTLocalEntry*> processors;
    uint64_t busFrequency;
    // the target entrypoint of the current CPU that is being booted
    void (*_entrypoint)();

    // Whether or not the APIC has been enabled
    bool _enabled = false;

    // A flag to check if the other processor has been started
    bool _cpuReady = false;

    bool enabled() {
        return _enabled;
    }

    void endInterrupt(u32 interrupt) {
        UNUSED(interrupt);
        registers[APIC_EOI_REGISTER] = 0;
    }

    void Init() {
        MADT* madt = (MADT*)findHeader("APIC");
        if (madt == nullptr) {
            LOG_ERROR("No MADT Header found.");
            return;
        }
        printf("APIC controller address at %X\n", madt->localControllerAddress);

        // First memory map the registers physical page to a virtual page
        memory::physicalMemoryManager.reservePhysicalMemory((void*)madt->localControllerAddress, 0x1000);
        registers = (uint32_t*)memory::kernelPageDirectory.bindPhysicalPage((void*)madt->localControllerAddress, memory::UserMode::Supervisor, KERNEL_END_VIRTUAL);

        // We remap the PIC , so they don't overlap with our PICs, which will replace them
        mapPics(0xE9, 0xF7);

        // Then we disable the PIC by masking all IRQs
        outb(SLAVE_DATA, 0xFF);
        outb(MASTER_DATA, 0xFF);

        // We enable the APIC by setting the spurious interrupt vector register
        registers[APIC_SIV_REGISTER] = 0x1FF;
        _enabled = true;

        // Disable the timer interrupt for now, and also set both lintx to 0, they will be configured later
        registers[APIC_LAPIC_TIMER_REGISTER] = 1<<16;
        registers[APIC_LINT0_REGISTER] = 0;
        registers[APIC_LINT1_REGISTER] = 0;

        // Then we iterate over each entry in the madt table
        MADTEntryHeader* entry = &madt->firstEntry;
        while ((u32)entry < (u32)madt + madt->header.Length) {
            if (entry->type == 0) {
                LOG_INFO("Found processor %d, apic %d, flags: %X", ((MADTLocalEntry*)entry)->processorId, ((MADTLocalEntry*)entry)->apicId, ((MADTLocalEntry*)entry)->flags);
                processors.push_back((MADTLocalEntry*)entry);
            } else if (entry->type == 1) {
                MADTIoEntry* ioentry = (MADTIoEntry*)entry;
                LOG_INFO("found IO APIC %d", ((MADTIoEntry*)entry)->apicId);
                ioApics.push_back(ioentry);
                // We also make sure that the registers are addressable
                if (memory::kernelPageDirectory.getVirtualAddress((void*) ioentry->apicAddress) != 0) {
                    ioentry->apicAddress = (uint32_t*)memory::kernelPageDirectory.getVirtualAddress((void*) ioentry->apicAddress);
                } else {
                    memory::physicalMemoryManager.reservePhysicalMemory((void*) ioentry->apicAddress, 8);
                    ioentry->apicAddress = (uint32_t*)memory::kernelPageDirectory.bindPhysicalPage((void*) ioentry->apicAddress, memory::UserMode::Supervisor, KERNEL_END_VIRTUAL);
                }
                printf("Mapped to %X\n", ioentry->apicAddress);
            } else if (entry->type == 4) {
                MADTNonMaskableInterruptsEntry* nmi = (MADTNonMaskableInterruptsEntry*)entry;
                LOG_INFO("found NMI %d", nmi->header.type);
                // configure it in the appropriate processors. This is running on the bootstrap processor so we only enable nmi's for processor 0
                if (nmi->processor == 0xFF || nmi->processor == 0) {
                    int target = nmi->lint == 0 ? APIC_LINT0_REGISTER : APIC_LINT1_REGISTER;
                    bool lowTriggered = (nmi->flags & 2) != 0;
                    bool levelTriggered = (nmi->flags & 8) != 0;
                    registers[target] = (0xFDu + nmi->lint) + (lowTriggered << 13) + (levelTriggered << 15) + (1 << 11);
                }
            } else if (entry->type == 2) {
                // For now we ignore source overrides
                MADTSourceOverrideEntry* override = (MADTSourceOverrideEntry*)entry;
                LOG_INFO("found source override %d %d %d %d", override->busSource, override->irqSource, override->globalSystemInterrupt, override->flags);
            } else {
                LOG_INFO("unhandled MADT entry type %d", entry->type);
            }
            entry = (MADTEntryHeader*)((char*)entry + entry->length);
        }

        // Now we are going to map all IO interrupts to 0x20 - 0x3F (32 to 64), like they would be mapped with the PIC
        for (uint32_t i = 0 ; i < ioApics.size() ; i++) {
            MADTIoEntry volatile* apic = ioApics[i];
            LOG_INFO("Mapping IOAPIC %X at address", apic->apicAddress, i);
            
            // and set their taskpriority to 0
            registers[APIC_TASKPRIOR] = 0;
            LOG_INFO("Set task priority\n");

            // We get the # of irqs this apic can handle
            apic->apicAddress[APIC_IO_SEL] = APIC_IO_VER_OFFSET;
            volatile unsigned int maxIrqs = ((apic->apicAddress[APIC_IO_WIN] & 0x00FF0000) >> 16) + 1;
            LOG_INFO("Max irqs: %d", maxIrqs);

            // and redirect each to globalBase + irq + 0x20
            for (uint32_t irq = 0 ; irq < maxIrqs ; irq++) {
                uint32_t lower = ((unsigned char)(irq + apic->globalBase + 0x20)) ; // all other flags happen to be 0
                uint32_t higher = 0; // needs to contain the cpuid in bit 56-63, but this is gonna be 0
                //LOG_INFO("setting ioapic %d %d", lower, higher);
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + irq*2;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = lower;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION1_OFFSET + irq*2;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = higher;
            }
        }

        // Lastly, we are going to set the timer. First, we initialize the timer
        for (int i = 0 ; i < 2 ; i++) {
            registers[APIC_TIMER_DIVIDE_REGISTER] = 0b1011;
            registers[APIC_TIMER_INITIAL_COUNT_REGISTER] = 0xFFFFFFFFu;
            registers[APIC_LAPIC_TIMER_REGISTER] = (1 << 16) | INT_PREEMPT;

            // Then we wait until the next second begins
            outb(0x70, 0x00);
            int curSecond = inb(0x71);
            int now;
            do {
                outb(0x70, 0x00);
                now = inb(0x71);
            } while (now != curSecond);

            // we reset the timer count
            registers[APIC_TIMER_INITIAL_COUNT_REGISTER] = 0xFFFFFFFFu;

            // and wait for the next second again
            do {
                outb(0x70, 0x00);
                curSecond = inb(0x71);
            } while (now == curSecond);

            // We read the elapsed ticks in 1 second, from this we can calculate the bus frequency.
            uint32_t count = registers[APIC_TIMER_CURRENT_COUNT_REGISTER];
            busFrequency = (0xFFFFFFFFu - count);
            LOG_INFO("External bus frequency: %u Mhz", (int) (busFrequency / 1024) / 1024);
            registers[APIC_TIMER_INITIAL_COUNT_REGISTER] = 0;
        }

        // and we initialize the gdt offset in the smp trampoline to our gdt
        smp_gdt_size = GdtSize();
        smp_gdt_offset = GdtOffset();
        smp_page = (memory::PageDirectory*)((u32)&memory::kernelPageDirectory - 0xC0000000);
    }

    void setSleep(uint8_t interrupt, uint32_t count, bool onetime) {
        registers[APIC_LAPIC_TIMER_REGISTER] = interrupt | (onetime ? 0 : 0x20000);
        registers[APIC_TIMER_INITIAL_COUNT_REGISTER] = count;
    }

    void StartAllCpus(void (*entrypoint)()) {
        _entrypoint = entrypoint;
        u32 startPage = ((size_t)0x8000 / 0x1000) & 0xFF;
        LOG_INFO("Addr: %X", 0x8000);
        LOG_INFO("Startpage: %X", startPage);

        for (u32 i = 0 ; i < processors.size() ; i++) {
            // skip processor 0, that's us :)
            if (processors[i]->processorId == 0)
                continue;

            // Allocate a stack for the processor
            smp_stack = new char[0x1000*16] + 0x1000*16;

            // Make sure the done flag is not set
            _cpuReady = false;

            // Send an init IPI
            registers[APIC_INT_COMMAND2_REGISTER] = processors[i]->apicId << 24;
            registers[APIC_INT_COMMAND1_REGISTER] = (5 << 8) | (1 << 14);
            hardSleep(15);

            // Then send two startup IPIs to actually boot up the other processor
            registers[APIC_INT_COMMAND2_REGISTER] = processors[i]->apicId << 24;
            registers[APIC_INT_COMMAND1_REGISTER] = startPage | (6 << 8) | (1 << 14);
            hardSleep(1);

            // We keep waiting while the other cpu is not ready yet
            while(!_cpuReady) {
                hardSleep(100);
            }
        }

        // and then we divide all interrupts over all processors
        for (uint32_t i = 0 ; i < ioApics.size() ; i++) {
            MADTIoEntry volatile* apic = ioApics[i];
            
            // and in the meantime also set its taskpriority to 0
            ((uint32_t volatile*) apic->apicAddress)[APIC_TASKPRIOR] = 0;

            // We get the # of irqs this apic can handle
            apic->apicAddress[APIC_IO_SEL] = APIC_IO_VER_OFFSET;
            unsigned int maxIrqs = ((apic->apicAddress[APIC_IO_WIN] & 0x00FF0000) >> 16) + 1;

            // and redirect each to globalBase + irq + 0x20
            for (uint32_t irq = 0 ; irq < maxIrqs ; irq++) {
                uint32_t lower = ((unsigned char)(irq + apic->globalBase + 0x20)); // all other flags happen to be 0
                uint32_t higher = (irq % processors.size()) << 24; // needs to contain the cpuid in bit 56-63, but this is gonna be 0
                //LOG_INFO("setting ioapic %d %d", lower, higher);
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + irq*2;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = lower;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION1_OFFSET + irq*2;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = higher;
            }
        }
    }

    void disableIrq(u8 irq)
    {
        // IRQ's are mapped starting from 0x20, so irq 0x21 has offset 1
        irq -= 0x20;

        // Read the current lower half of the irq
        ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + irq*2;
        u32 lower = ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_WIN];

        // set its disable bit
        lower |= 1<<16;

        // and write the updated register
        ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + irq*2;
        ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_WIN] = lower;
    }

    void enableIrq(u8 irq)
    {
        // IRQ's are mapped starting from 0x20, so irq 0x21 has offset 1
        irq -= 0x20;

        // Read the current lower half of the irq
        ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + irq*2;
        u32 lower = ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_WIN];

        // unset its disable bit
        lower &= ~(1<<16);

        // and write the updated register
        ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + irq*2;
        ((uint32_t volatile*) ioApics[0]->apicAddress)[APIC_IO_WIN] = lower;
    }

    void wakeupOtherCpus()
    {
        // send an IPI to all other CPUS
        registers[APIC_INT_COMMAND2_REGISTER] = 0;
        registers[APIC_INT_COMMAND1_REGISTER] = 0xFE | (3 << 18);

        // and wait for the delivery status to confirm the interrupt
        while (registers[APIC_INT_COMMAND1_REGISTER] & (1 << 12)) {
            __asm__("pause");
        }
    }

    void wakeupAllCpus()
    {
        // send an IPI to all other CPUS
        registers[APIC_INT_COMMAND2_REGISTER] = 0;
        registers[APIC_INT_COMMAND1_REGISTER] = 0xFE | (2 << 18);

        // and wait for the delivery status to confirm the interrupt
        while (registers[APIC_INT_COMMAND1_REGISTER] & (1 << 12)) {
            __asm__("pause");
        }
    }

    void wakeupOneCpu(u8 id)
    {
        // send an IPI to all other CPUS
        registers[APIC_INT_COMMAND2_REGISTER] = id << 24;
        registers[APIC_INT_COMMAND1_REGISTER] = 0xFE;

        // and wait for the delivery status to confirm the interrupt
        while (registers[APIC_INT_COMMAND1_REGISTER] & (1 << 12)) {
            __asm__("pause");
        }
    }

    u8 id()
    {
        cpuid_field id = cpuid(1);
        return id.ebx >> 24;
    }
}

void SmpEntryPoint() {
    using namespace apic;
    LOG_INFO("greetings from cpu %X", id());

    // TODO at some point the core will needs his own TSS

    // Load the IDT on this cpu
    IdtFlush();

    // re-enable interrupts
    __asm__ volatile("sti");

    // re-enable NMI
    u8 flags = inb(0x70);
    flags |= 0x80;
    outb(0x70, flags);

    // Enable the APIC of this CPU
    registers[APIC_SIV_REGISTER] = 0x1FF;
    
    _cpuReady = true;
    _entrypoint();
    LOG_ERROR("CPU %d halted: reached end of execution.", id());

    while(true) {
        __asm__("hlt");
    }
}