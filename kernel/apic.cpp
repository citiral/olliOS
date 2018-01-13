#include "acpi.h"
#include "apic.h"
#include "memory/virtual.h"
#include "memory/physical.h"
#include "pic.h"
#include "io.h"
#include "kstd/vector.h"
#include "sleep.h"
#include "cdefs.h"
#include <stdio.h>

extern "C" void smp_trampoline_entry();

using namespace acpi;

namespace apic {

    // A pointer to the registers of the APIC. These are by default mapped to physical page FEE00xxx
    uint32_t volatile* registers;
    std::vector<MADTIoEntry*> ioApics;
    std::vector<MADTLocalEntry*> processors;
    uint64_t busFrequency;

    bool _enabled = false;

    bool enabled() {
        return _enabled;
    }

    void endInterrupt(u32 interrupt) {
        UNUSED(interrupt);
        registers[APIC_EOI_REGISTER] = 1;
    }

    void Init() {
        // First memory map the registers physical page to a virtual page
        physicalMemoryManager.reservePhysicalMemory((void*)0xFEE00000, 0x1000);
        registers = (uint32_t*)kernelPageDirectory.bindPhysicalPage((void*)0xFEE00000);

        // We remap the PIC , so they don't overlap with our PICs, which will replace them
        mapPics(0xE9, 0xF7);

        // Then we disable the PIC by masking all IRQs
        outb(SLAVE_DATA, 0xFF);
        outb(MASTER_DATA, 0xFF);

        //outb(0x22, 0x70);
        //outb(0x23, 0x1);

        // We enable the APIC by setting the spurious interrupt vector register
        registers[APIC_SIV_REGISTER] = 0x1FF;
        _enabled = true;

        // Disable the timer interrupt for now, and also set both lintx to 0, they will be configured later
        registers[APIC_LAPIC_TIMER_REGISTER] = 1<<16;
        registers[APIC_LINT0_REGISTER] = 0;
        registers[APIC_LINT1_REGISTER] = 0;

        MADT* madt = (MADT*)findHeader("APIC");
        if (madt == nullptr) {
            LOG_ERROR("No MADT Header found.");
            // Reenable the irqs if we fail
            outb(SLAVE_DATA, 0);
            outb(MASTER_DATA, 0);
            mapPics(0x20, 0x28);
        }

        // Then we iterate over each entry in the madt table
        MADTEntryHeader* entry = &madt->firstEntry;
        while ((u32)entry < (u32)madt + madt->header.Length) {
            if (entry->type == 0) {
                LOG_INFO("Found processor %d, apic %d, flags: %X", ((MADTLocalEntry*)entry)->processorId, ((MADTLocalEntry*)entry)->apicId, ((MADTLocalEntry*)entry)->flags);
                processors.push_back((MADTLocalEntry*)entry);
            } else if (entry->type == 1) {
                MADTIoEntry* ioentry = (MADTIoEntry*)entry;
                ioApics.push_back(ioentry);
                // We also make sure that the registers are addressable
                if (kernelPageDirectory.getVirtualAddress(ioentry->apicAddress) != 0) {
                    ioentry->apicAddress = (uint32_t*)kernelPageDirectory.getVirtualAddress(ioentry->apicAddress);
                } else {
                    physicalMemoryManager.reservePhysicalMemory(ioentry->apicAddress, 8);
                    ioentry->apicAddress = (uint32_t*)kernelPageDirectory.bindPhysicalPage(ioentry->apicAddress);
                }
                LOG_INFO("found IO APIC %d", ((MADTIoEntry*)entry)->apicId);
            } else if (entry->type == 4) {
                MADTNonMaskableInterruptsEntry* nmi = (MADTNonMaskableInterruptsEntry*)entry;
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
            // We get the # of irqs this apic can handle
            apic->apicAddress[APIC_IO_SEL] = APIC_IO_VER_OFFSET;
            unsigned int maxIrqs = ((apic->apicAddress[APIC_IO_WIN] & 0x00FF0000) >> 16) + 1;

            // and redirect each to globalBase + irq + 0x20
            for (uint32_t irq = 0 ; irq < maxIrqs ; irq++) {
                uint32_t lower = ((unsigned char)(irq + apic->globalBase + 0x20)); // all other flags happen to be 0
                uint32_t higher = 0; // needs to contain the cpuid in bit 56-63, but this is gonna be 0
                //LOG_INFO("setting ioapic %d %d", lower, higher);
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + irq*2;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = lower;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION1_OFFSET + irq*2;
                ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = higher;
            }
        }

        // Lastly, we are going to set the timer. First, we initialize the timer
        registers[APIC_TIMER_DIVIDE_REGISTER] = 11;
        registers[APIC_TIMER_INITIAL_COUNT_REGISTER] = 0xFFFFFFFFu;
        registers[APIC_LAPIC_TIMER_REGISTER] = 0xFC;

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
        busFrequency = (0xFFFFFFFFu - count) * 16;
        LOG_INFO("External bus frequency: %u Mhz", (int) (busFrequency / 1024) / 1024);
        registers[APIC_TIMER_INITIAL_COUNT_REGISTER] = 0;
    }

    void setSleep(uint32_t count, bool onetime) {
        registers[APIC_LAPIC_TIMER_REGISTER] = 0xFCu | (onetime ? 0 : (1<<17));
        registers[APIC_TIMER_INITIAL_COUNT_REGISTER] = count;
    }

    void StartAllCpus(void* startAddress) {
        return;
        UNUSED(startAddress);
        u32 startPage = ((u32)smp_trampoline_entry / 0x1000) & 0xFF;
        LOG_INFO("Addr: %X", smp_trampoline_entry );
        LOG_INFO("Startpage: %X", startPage);

        for (int i = 0 ; i < 2 ; i++) {
            // skip processor 0, that's us :)
            if (processors[i]->processorId == 0)
                continue;

            // First send an init IPI
            registers[APIC_INT_COMMAND2_REGISTER] = processors[i]->apicId << 24;
            registers[APIC_INT_COMMAND1_REGISTER] = (5 << 8) | (1 << 14);
            sleep(15);

            // Then send two startup IPIs to actually boot up the other processor
            registers[APIC_INT_COMMAND2_REGISTER] = processors[i]->apicId << 24;
            registers[APIC_INT_COMMAND1_REGISTER] = startPage | (6 << 8) | (1 << 14);
            sleep(1);

            //registers[APIC_INT_COMMAND2_REGISTER] = processors[i]->apicId << 24;
            //registers[APIC_INT_COMMAND1_REGISTER] = startPage | (6 << 8) | (1 << 14);
            //sleep(1);
        }
    }
}