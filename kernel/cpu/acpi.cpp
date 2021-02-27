#include "acpi.h"
#include "cdefs.h"
#include "linker.h"
#include <stdio.h>
#include <string.h>
#include "memory/physical.h"
#include "memory/virtual.h"

namespace acpi {
    RSDPDescriptor* rsdp;
    RSDT* rsdt;

    void init() {
        // First we have to find the RSDP
        bool found = false;
        for (char* i = (char*)0x000E0000 ; i < (char*)0x000FFFFF - sizeof(RSDPDescriptor) ; i++) {
            if (memcmp(i, "RSD PTR ", 8) == 0) {
                found = true;
                rsdp = (RSDPDescriptor*)i;
                break;
            }
        }

        if (!found) {
            LOG_ERROR("Unable to find RSDP.");
            return;
        }

        if (!validateChecksum((unsigned char*)rsdp, sizeof(RSDPDescriptor))) {
            LOG_ERROR("RSDP checksum invalid.");
            return;
        }

        rsdt = (RSDT*)mapHeader((ACPISDTHeader*)rsdp->RsdtAddress);
        LOG_INFO("resdt size %d", rsdt->header.Length);
        if (!validateChecksum((unsigned char*)rsdt, rsdt->header.Length)) {
            LOG_ERROR("RSDT checksum invalid.");
            return;
        }
    }

    bool validateChecksum(unsigned char* data, unsigned int length) {
        unsigned int checksum = 0;
        for (unsigned int i = 0 ; i < length ; i++) {
            checksum += data[i];
        }
        return (char)checksum == 0;
    }

    ACPISDTHeader* findHeader(const char signature[4]) {
        for (unsigned int i = 0 ; i < (rsdt->header.Length - sizeof(rsdt->header)) / sizeof(ACPISDTHeader*) ; i += 1) {
            if (rsdt->headers[i] != nullptr) {
                ACPISDTHeader* virt = mapHeader(rsdt->headers[i]);
                if (memcmp(virt, signature, 4) == 0)
                    return virt;                
            }
        }
        return nullptr;
    }

    ACPISDTHeader* mapHeader(ACPISDTHeader* header) {
        ACPISDTHeader* result;

        // if it's already mapped, do nothing
        if (memory::kernelPageDirectory.getVirtualAddress(header) != 0) {
            return (ACPISDTHeader*)memory::kernelPageDirectory.getVirtualAddress(header);
        }
        
        // First we reserve the header's physical memory and map it to some virtual memory
        memory::physicalMemoryManager.reservePhysicalMemory(header, sizeof(ACPISDTHeader));
        result = (ACPISDTHeader*)((u32)memory::kernelPageDirectory.bindPhysicalPage((char*)((u32)header & 0xFFFFF000), memory::UserMode::Supervisor, KERNEL_END_VIRTUAL) + ((u32)header & 0x00000FFF));

        // Now we can read the length of the header, so we can reserve the rest
        memory::physicalMemoryManager.reservePhysicalMemory((char*)header + sizeof(ACPISDTHeader), result->Length);
        
        // And for now we just hope the rest of the data is contained in the page we just allocated :)
        // If this is not the case, it'll crash and i'll have to implement that

        return result;
    }
}