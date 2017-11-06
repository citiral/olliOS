#include "acpi.h"
#include "apic.h"
#include "stdint.h"
#include "memory/virtual.h"
#include "memory/physical.h"
#include "pic.h"
#include "io.h"
#include "kstd/vector.h"
#include "cdefs.h"
#include <stdio.h>

using namespace acpi;

namespace apic {

    // A pointer to the registers of the APIC. These are by default mapped to physical page FEE00xxx
    uint32_t volatile* registers;
    std::vector<MADTIoEntry*> ioApics;
    std::vector<MADTLocalEntry*> processors;

    bool _enabled = false;

    bool enabled() {
        return _enabled;
    }

    void endInterrupt(u32 interrupt) {
        registers[APIC_EOI_REGISTER] = 1;
    }

    void Init() {
        // First memory map the registers physical page to a virtual page
        physicalMemoryManager.reservePhysicalMemory((void*)0xFEE00000, 0x1000);
        registers = (uint32_t*)kernelPageDirectory.bindPhysicalPage((void*)0xFEE00000);

        // We remap the PIC , so they don't overlap with our PICs, which will replace them
        mapPics(0xE9, 0xF7);

        // Then we disable the PIC by masking all IRQs
        outb(SLAVE_DATA, 0xFFFF);
        outb(MASTER_DATA, 0xFFFF);

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

                /*bool lowTriggered = (override->flags & 2) != 0;
                bool levelTriggered = (override->flags & 8) != 0;

                // We find the apic that has this as base
                for (uint32_t i = 0 ; i < ioApics.size() ; i++) {
                    MADTIoEntry volatile* apic = ioApics[i];
                    apic->apicAddress[APIC_IO_SEL] = APIC_IO_VER_OFFSET;
                    unsigned int maxIrqs = (apic->apicAddress[APIC_IO_WIN] & 0x00FF0000) >> 16;

                    if (apic->globalBase <= override->irqSource && apic->globalBase + maxIrqs > override->irqSource) {
                        LOG_DEBUG("addr %X", apic->apicAddress);
                        ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION0_OFFSET + (override->irqSource - apic->globalBase)*2;
                        ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = override->globalSystemInterrupt + (lowTriggered << 13) + (levelTriggered << 15);
                        ((uint32_t volatile*) apic->apicAddress)[APIC_IO_SEL] = APIC_IO_REDIRECTION1_OFFSET + (override->irqSource - apic->globalBase)*2;
                        ((uint32_t volatile*) apic->apicAddress)[APIC_IO_WIN] = 0;

                        LOG_INFO("Setting irq %d %d", override->globalSystemInterrupt + (lowTriggered << 13) + (levelTriggered << 15), 0);
                    }
                }*/
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
        BOCHS_BREAKPOINT

        LOG_INFO("error status: %X", registers[APIC_ERROR_REGISTER]);
    }

}