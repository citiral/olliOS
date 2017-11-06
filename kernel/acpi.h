#ifndef __OLLIOS_ACPI_H
#define __OLLIOS_ACPI_H

#include <stdint.h>

struct RSDPDescriptor {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__ ((packed));

struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__ ((packed));

struct RSDT {
    ACPISDTHeader header;
    ACPISDTHeader* headers[];
};

namespace acpi {
    extern RSDPDescriptor* rsdp;
    extern RSDT* rsdt;

    void init();
    bool validateChecksum(unsigned char* data, unsigned int length);
    ACPISDTHeader* findHeader(const char signature[4]);
    // Makes sure the given header + data are addressable. Returns the addressable location in virtual memory
    ACPISDTHeader* mapHeader(ACPISDTHeader* header);
    
}
#endif