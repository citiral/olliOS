#ifndef __OLLIOS_ACPI_H
#define __OLLIOS_ACPI_H

#include <stdint.h>

namespace acpi {    

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
    
    struct ACPIAdressStructure
    {
        uint8_t address_space_id;    // 0 - system memory, 1 - system I/O
        uint8_t register_bit_width;
        uint8_t register_bit_offset;
        uint8_t reserved;
        uint64_t address;
    } __attribute__((packed));

    struct HPETHeader
    {
        ACPISDTHeader header;
        uint8_t hardware_rev_id;
        uint8_t comparator_count:5;
        uint8_t counter_size:1;
        uint8_t reserved:1;
        uint8_t legacy_replacement:1;
        uint16_t pci_vendor_id;
        ACPIAdressStructure address;
        uint8_t hpet_number;
        uint16_t minimum_tick;
        uint8_t page_protection;
    } __attribute__((packed));
    
    struct RSDT {
        ACPISDTHeader header;
        ACPISDTHeader* headers[];
    } __attribute__ ((packed));
    
    struct MADTEntryHeader {
        uint8_t type;
        uint8_t length;
    } __attribute__ ((packed));
    
    struct MADTLocalEntry {
        MADTEntryHeader header;
        uint8_t processorId;
        uint8_t apicId;
        uint32_t flags;
    } __attribute__ ((packed));
    
    struct MADTIoEntry {
        MADTEntryHeader header;
        uint8_t apicId;
        uint8_t reserved;
        volatile uint32_t* apicAddress;
        uint32_t globalBase;
    } __attribute__ ((packed));
        
    struct MADTSourceOverrideEntry {
        MADTEntryHeader header;
        uint8_t busSource;
        uint8_t irqSource;
        uint32_t globalSystemInterrupt;
        uint16_t flags;
    } __attribute__ ((packed));

    struct MADTNonMaskableInterruptsEntry {
        MADTEntryHeader header;
        uint8_t processor;
        uint16_t flags;
        uint8_t lint;
    } __attribute__ ((packed));
    
    struct MADTLocalAPICAddressOverrideEntry {
        MADTEntryHeader header;
        uint16_t reserved;
        uint64_t physicalAddress;
    } __attribute__ ((packed));

    struct MADT {
        ACPISDTHeader header;
        uint32_t localControllerAddress;
        uint32_t flags;
        MADTEntryHeader firstEntry;
    } __attribute__ ((packed));

    extern RSDPDescriptor* rsdp;
    extern RSDT* rsdt;

    void init();
    bool validateChecksum(unsigned char* data, unsigned int length);
    ACPISDTHeader* findHeader(const char signature[4]);
    // Makes sure the given header + data are addressable. Returns the addressable location in virtual memory
    ACPISDTHeader* mapHeader(ACPISDTHeader* header);
    
}
#endif