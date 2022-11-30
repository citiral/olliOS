#include "types.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "linker.h"
#include "multiboot.h"

#include "cpu/acpi.h"
#include "cpu/cpu.h"
#include "cpu/gdt.h"
#include "cpu/tss.h"
#include "cpu/pic.h"
#include "cpu/apic.h"
#include "cpu/io.h"
#include "cpu/cpuid.h"
#include "cpu/pit.h"
#include "cpu/hpet.h"
#include "cpu/cmos.h"

#include "memory/alloc.h"
#include "memory/virtual.h"
#include "memory/physical.h"

#include "threading/scheduler.h"

#include "elf/symbolmap.h"
#include "elf/elf.h"

#include "vga/vga.h"

#include "file.h"
#include "virtualfile.h"
#include "threading/semaphore.h"

VgaDriver* vgaDriver = nullptr;

extern void *__realmode_lma_start;
extern void *__realmode_lma_end;
extern void *__realmode_vma_start;

extern "C" void test_boop(void);

void initCpu() {
    // setup a flat segmentation structure. We're going to be using paging anyway.
	GdtCreateFlat();
	GdtFlush();
    LOG_STARTUP("GDT initialized.");

    // Then we set up the interupt tables so we can actually use interrupts (these contain function pointers to interrupt handlers)
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
    LOG_STARTUP("IDT initialized.");

    // Program the PIC so interrupts make sense (0-32 is reserved for intel, ...)
	PicInit();
    LOG_STARTUP("PIC initialized.");

    // Initialize the PIT to a known state
    pit::init();
}

void initMemory(multiboot_info* multiboot) {
    // We initialise the state of the physical memory allocator
    memory::physicalMemoryManager.init();

    // use the multiboot header to discover valid memory regions
    if ((multiboot->flags & (1 << 6)) == 0) {
        // if multiboot didn't pass us our usable memory areas (which it really always should) use a default.
        LOG_STARTUP("Err: No MMAP specified by multiboot.");
        kernelAllocator.init((void*)KERNEL_END_VIRTUAL, 0xFFFFFFFF - (size_t)KERNEL_END_VIRTUAL);

        // set up the physical memory allocator from the end of the kernel to the end of the address space
        memory::physicalMemoryManager.registerAvailableMemory(KERNEL_END_PHYSICAL, 0xFFFFFFFF);
    } else {
        // loop over each mmap descriptor, these are of variable size so the loop is special
        for (multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)multiboot->mmap_addr;
             (u64)mmap != (u64)multiboot->mmap_addr + multiboot->mmap_length;
             mmap = (multiboot_memory_map_t*)((char*)mmap + mmap->size + sizeof(mmap->size))) {

            void* addr = (void*)mmap->addr;
            size_t length = (size_t)mmap->len;

            // only use it if type is 1 (usable RAM)
            if (mmap->type != 1)
                continue;

            // don't let this shit overlap with the kernel itself
            if ((size_t)addr < (size_t)KERNEL_END_PHYSICAL) {
                size_t diff = (size_t)KERNEL_END_PHYSICAL - (size_t) addr;
                addr = (void*)((size_t)addr + diff);

                if (length < diff)
                    continue;

                length -= diff;
            }

            // and let the physical memory manager know that memory is free.
            // we have to round it up the the nearest page though
            u32 offset = (0x1000 - (u32)addr % 0x1000);
            LOG_INFO("reserving %lX bytes starting at %lX.", length - offset, (size_t) addr + offset);
            memory::physicalMemoryManager.registerAvailableMemory((void*) ((size_t) addr + offset), length - offset);
        }
    }

    // make sure the memory of the other modules isn't overwritten
    multiboot_module_t *mod = (multiboot_module_t*) multiboot->mods_addr;
    for (multiboot_uint32_t i = 0 ; i < multiboot->mods_count ; i++) {
        memory::physicalMemoryManager.reservePhysicalMemory((void*) mod[i].mod_start, mod[i].mod_end - mod[i].mod_start);
    }

    // Now that we can allocate physical memory, initialize the paging. This should set up a valid not 4MB pagetable, with only the KERNEL_BEGIN_PHYSICAL to KERNEL_END_PHYSICAL mapped to the higher half, and nothing more.
    memory::initializePaging();
    for (multiboot_uint32_t i = 0 ; i < multiboot->mods_count ; i++) {
        memory::kernelPageDirectory.mapMemory((void*) mod[i].mod_start, (void*) mod[i].mod_start, mod[i].mod_end - mod[i].mod_start, memory::UserMode::Supervisor);
    }

    LOG_STARTUP("Paging initialized.");
}

void cpu_main() {
    // Allocate a TSS for the cpu
    tss::TaskStateSegment* tss = tss::allocate_tss();

    // Add it to the GDT
    u32 index = GdtAddTss(tss);
    GdtFlush();

    // And use it on this CPU
    tss::use_tss_at_index(index);
    threading::set_tss_pointer(tss);

    apic::setSleep(INT_SPURIOUS, 1024*1024*64, false);
    
    while (true) {
        vgaDriver->setChar('!', VGA_WIDTH - 1, 0);
        if (!threading::scheduler->enter()) {
            vgaDriver->setChar('.', VGA_WIDTH - 1, 0);
            __asm__ ("hlt");
            vgaDriver->setChar('#', VGA_WIDTH - 1, 0);
        }
    }
}

extern "C" void main(multiboot_info* multiboot) {
    // Store the multiboot header so it's accessible globally
    multiboot::instance = multiboot;

    // init lowlevel CPU related stuff
    // None of these should be allowed to touch the memory allocator, etc
	initCpu();

    // init the memory management, so we have proper paging and can allocate memory
    initMemory(multiboot);

    fs::init();
    fs::root->create("sys", FILE_CREATE_DIR);
    fs::root->create("dev", FILE_CREATE_DIR);

    /* Each of these __realmode* values is generated by the linker script */
    uint32_t *src_addr = (uint32_t *)&__realmode_lma_start;
    uint32_t *dst_addr = (uint32_t *)&__realmode_vma_start;
    uint32_t *src_end  = (uint32_t *)&__realmode_lma_end;

    /* Copy a DWORD at a time from source to destination */
    while (src_addr < src_end)
        *dst_addr++ = *src_addr++;

    acpi::init();

    //set up pre-emptive multithreading
    idt.getEntry(INT_SPURIOUS).setOffset((u32)thread_interrupt);
    threading::scheduler = new threading::Scheduler();

    vgaDriver = new VgaDriver(multiboot);
    fs::root->get("dev")->bind(vgaDriver);

    printf("Flags: %X\n", multiboot->flags);
    printf("mods count: %d\n", multiboot->mods_count);
    multiboot_module_t *mod = (multiboot_module_t*) multiboot->mods_addr;
    
    printf("elf: %d\n", multiboot->u.elf_sec.size);
    symbolMap = new SymbolMap((const char*) mod[0].mod_start);
    printf("symbol map build.\n");

    // if APIC is supported, switch to it and enable multicore
    cpuid_field features = cpuid(1);
    if ((features.edx & (int)cpuid_feature::EDX_APIC) != 0) {
        LOG_STARTUP("Initializing APIC.");
        apic::Init();   
        //apic::StartAllCpus(&cpu_main);
    } else {
        LOG_STARTUP("APIC not supported, skipping. (Threading will not be supported)");
    }

    if (!hpet::hpet.init()) {
        printf("Failed initializing HPET.\n");
    }

    printf("Found %d modules\n", multiboot->mods_count);
    for (multiboot_uint32_t i = 2 ; i < multiboot->mods_count ; i++) {
        printf("loading module %d at 0x%X\n", i, mod[i].mod_start);
        u8* c = (u8*) (mod[i].mod_start);
        u8* data = new u8[mod[i].mod_end - mod[i].mod_start];
        memcpy(data, c, mod[i].mod_end - mod[i].mod_start);

        elf::elf* e = new elf::elf(data, true);
        if (e == nullptr) {
            printf("Failed allocating elf\n");
            while (1);
        }

        if (e->link_as_kernel_module(*symbolMap) != 0 && 0) {
            printf("failed linking elf\n");
        } else {
            void (*module_load)(fs::File*, const char*);
            e->get_symbol_value("module_load", (u32*) &module_load);
            module_load(fs::root, (const char*) mod[i].cmdline);
        }
        //memory::kernelPageDirectory.unbindVirtualPage((void*) mod[i].mod_start, mod[i].mod_end - mod[i].mod_start);
        //memory::physicalMemoryManager.freePhysicalMemory((void*) mod[i].mod_start, mod[i].mod_end - mod[i].mod_start);
    }

    cpu_main();
}
