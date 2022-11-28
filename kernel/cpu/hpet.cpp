#include "hpet.h"
#include "acpi.h"
#include "stdio.h"
#include "types.h"
#include "../memory/virtual.h"

using namespace hpet;

#define GEN_CAP_VENDOR_ID                   0xFFFF0000
#define GEN_CAP_LEGACY_REPLACEMENT_CAPABLE  0x00008000
#define GEN_CAP_COUNTER_64_BIT_CAPABLE      0x00002000
#define GEN_CAP_COUNTER_AMOUNT              0x00001F00
#define GEN_CAP_REVISION_ID                 0x000000FF
#define GEN_CAP_VENDOR_ID_OFFSET                   16
#define GEN_CAP_LEGACY_REPLACEMENT_CAPABLE_OFFSET  15
#define GEN_CAP_COUNTER_64_BIT_CAPABLE_OFFSET      13
#define GEN_CAP_COUNTER_AMOUNT_OFFSET              8
#define GEN_CAP_REVISION_ID_OFFSET                 0

#define GEN_CONF_LEGACY_MAPPING_ENABLED 0x00000002
#define GEN_CONF_ENABLED                0x00000001
#define GEN_CONF_LEGACY_MAPPING_ENABLED_OFFSET 1
#define GEN_CONF_ENABLED_OFFSET                0


struct HpetTimerRegisters {
    uint64_t configuration;
    uint64_t comparator;
    uint64_t routing;
};


struct HpetRegisters {
    volatile uint32_t generalCapabilities;
    volatile uint32_t counterPeriod;

    volatile uint64_t _reserved1;

    volatile uint32_t generalConfig;
    volatile uint32_t _reserved2;

    volatile uint64_t _reserved3;

    volatile uint32_t interruptStatus;
    volatile uint32_t _reserved4;
    volatile uint32_t counter1;
    volatile uint32_t counter2;
    volatile HpetTimerRegisters timers[];
} __attribute__((packed));


struct HpetInstance {
    volatile HpetRegisters* registers; 
};

HpetInstance hpetInstance;


uint32_t mask_and_shift(uint64_t reg, uint64_t mask, uint64_t offset) {
    return (reg & mask) >> offset;
}


bool hpet::init()
{
    acpi::HPETHeader* hpet = (acpi::HPETHeader*)acpi::findHeader("HPET");
    if (hpet == nullptr) {
        return false;
    } else {
        acpi::mapHeader((acpi::ACPISDTHeader*) hpet);
    }
    
    printf("hardware_rev_id: %u\n", hpet->hardware_rev_id);
    printf("comparator_count: %u\n", hpet->comparator_count);
    printf("counter_size: %u\n", hpet->counter_size);
    printf("reserved: %u\n", hpet->reserved);
    printf("legacy_replacement: %u\n", hpet->legacy_replacement);
    printf("pci_vendor_id: %x\n", hpet->pci_vendor_id);
    //printf("address: %u\n", hpet->address);
    printf("hpet_number: %u\n", hpet->hpet_number);
    printf("minimum_tick: %u\n", hpet->minimum_tick);
    printf("page_protection: %u\n", hpet->page_protection);

    printf("address_space_id: %X\n", hpet->address.address_space_id);    // 0 - system memory, 1 - system I/O
    printf("register_bit_width: %X\n", hpet->address.register_bit_width);
    printf("register_bit_offset: %X\n", hpet->address.register_bit_offset);
    printf("reserved: %X\n", hpet->address.reserved);
    printf("address: %X\n", hpet->address.address);


    hpetInstance.registers = (HpetRegisters*) memory::kernelPageDirectory.bindPhysicalPage((void*) hpet->address.address, memory::UserMode::Supervisor);
    if (hpetInstance.registers == nullptr) {
        return false;
    }

    volatile uint64_t* r = (uint64_t*) hpetInstance.registers;

    for (int i = 0 ; i < 8 ; i++) {
        printf("%d: %x\n", i, r[i]);
    }

    printf("bound registers to %x\n", hpetInstance.registers);
    printf("gen cap: %x\n", hpetInstance.registers->generalCapabilities);
    printf("gen cap: %x\n", GEN_CAP_REVISION_ID);
    uint32_t a = hpetInstance.registers->generalCapabilities;
    printf("revisionId: %x\n", mask_and_shift(a, GEN_CAP_REVISION_ID, GEN_CAP_REVISION_ID_OFFSET));
    printf("timerAmount: %x\n", mask_and_shift(a, GEN_CAP_COUNTER_AMOUNT, GEN_CAP_COUNTER_AMOUNT_OFFSET));
    printf("64bit: %x\n", mask_and_shift(a, GEN_CAP_COUNTER_64_BIT_CAPABLE, GEN_CAP_COUNTER_64_BIT_CAPABLE_OFFSET));
    printf("legacy supported: %x\n", mask_and_shift(a, GEN_CAP_LEGACY_REPLACEMENT_CAPABLE, GEN_CAP_LEGACY_REPLACEMENT_CAPABLE_OFFSET));
    printf("vendorid: %x\n", mask_and_shift(a, GEN_CAP_VENDOR_ID, GEN_CAP_VENDOR_ID_OFFSET));
    printf("counterPeriod: %x\n", hpetInstance.registers->counterPeriod);
    /*printf("amountOfTimers: %d\n", hpetInstance.registers->generalCapabilities.amountOfTimers);
    printf("is64Bit: %d\n", hpetInstance.registers->generalCapabilities.is64Bit);
    printf("_reserved: %d\n", hpetInstance.registers->generalCapabilities._reserved);
    printf("legacyReplacementSupported: %d\n", hpetInstance.registers->generalCapabilities.legacyReplacementSupported);
    printf("vendorId: %d\n", hpetInstance.registers->generalCapabilities.vendorId);
    printf("counterPeriod: %d\n", hpetInstance.registers->generalCapabilities.counterPeriod);*/

    uint32_t gconf = hpetInstance.registers->generalConfig;
    gconf |= GEN_CONF_ENABLED;
    hpetInstance.registers->generalConfig = gconf;

    while (1) {
        printf("tim: %x\n", hpetInstance.registers->counter1);
        printf("tim: %x\n", hpetInstance.registers->counter2);
    }

    return true;
}

u64 hpet::get_elapsed_ns()
{
    return 0;
}

void hpet::wait(u64 duration, HpetCallback cb)
{

}
   
void hpet::interval(u64 interval, HpetCallback cb)
{

}
