#include "types.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "linker.h"
#include "multiboot.h"

#include "cpu/acpi.h"
#include "cpu/cpu.h"
#include "cpu/gdt.h"
#include "cpu/pic.h"
#include "cpu/apic.h"
#include "cpu/io.h"
#include "cpu/cpuid.h"

#include "memory/alloc.h"
#include "memory/virtual.h"
#include "memory/physical.h"

#include "threading/scheduler.h"

#include "elf/symbolmap.h"
#include "elf/elf.h"

#include "vga/vga.h"

#include "bindings.h"

VgaDriver* vgaDriver = nullptr;

extern void *__realmode_lma_start;
extern void *__realmode_lma_end;
extern void *__realmode_vma_start;

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

    // Initialize the TSS (needed for system calls)
	//initialize_tss(0x10, 0x28);
    //LOG_STARTUP("TSS initialized.");

    // Program the PIC so interrupts make sense (0-32 is reserved for intel, ...)
	PicInit();
    LOG_STARTUP("PIC initialized.");
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
        memory::physicalMemoryManager.reservePhysicalMemory((void*) mod->mod_start, mod->mod_end - mod->mod_start);
        mod++;
    }

    // Now that we can allocate physical memory, initialize the paging. This should set up a valid not 4MB pagetable, with only the KERNEL_BEGIN_PHYSICAL to KERNEL_END_PHYSICAL mapped to the higher half, and nothing more.
    memory::initializePaging();
    LOG_STARTUP("Paging initialized.");
}

void cpu_main() {
    apic::setSleep(INT_PREEMPT, apic::busFrequency / 1024, false);
    while (true) {
        threading::scheduler->enter();
        __asm__ ("pause");
    }
}


class test {
public:
    test(int v): _v(v) {
        printf("+ test %d\n", _v);
    }

    test(test& t): _v(t._v) {        
        printf("+ cp test %d\n", _v);
    }

    ~test() {
        printf("- test %d\n", _v);
    }

    test& operator=(const test& t) {
        _v = t._v;
        printf("= test %d\n", _v);
        return *this;
    }

    int _v;
};

extern "C" void main(multiboot_info* multiboot) {
    // init lowlevel CPU related stuff
    // None of these should be allowed to touch the memory allocator, etc
	initCpu();

    // init the memory management, so we have proper paging and can allocate memory
    initMemory(multiboot);

    bindings::init();
    bindings::root->add(new bindings::OwnedBinding("sys"));
    bindings::root->add(new bindings::OwnedBinding("dev"));

    /* Each of these __realmode* values is generated by the linker script */
    uint32_t *src_addr = (uint32_t *)&__realmode_lma_start;
    uint32_t *dst_addr = (uint32_t *)&__realmode_vma_start;
    uint32_t *src_end  = (uint32_t *)&__realmode_lma_end;

    /* Copy a DWORD at a time from source to destination */
    while (src_addr < src_end)
        *dst_addr++ = *src_addr++;

    acpi::init();

    //set up pre-emptive multithreading
    idt.getEntry(INT_PREEMPT).setOffset((u32)thread_interrupt);
    threading::scheduler = new threading::Scheduler();

    printf("Flags: %X\n", multiboot->flags);
    printf("mods count: %d\n", multiboot->mods_count);
    multiboot_module_t *mod = (multiboot_module_t*) multiboot->mods_addr;

    printf("elf: %d\n", multiboot->u.elf_sec.size);
    symbolMap = new SymbolMap((const char*) mod->mod_start);
    printf("symbol map build.\n");

    
    vgaDriver = new VgaDriver();
    bindings::root->get("sys")->add((new bindings::OwnedBinding("vga"))->on_write([](bindings::OwnedBinding* vga, size_t size, const void* data){
        (void) vga;
        vgaDriver->write(data, size);
        return true;
    }));

    // if APIC is supported, switch to it and enable multicore
    cpuid_field features = cpuid(1);
    if ((features.edx & (int)cpuid_feature::EDX_APIC) != 0) {
        LOG_STARTUP("Initializing APIC.");
        apic::Init();
        //apic::StartAllCpus(&cpu_main);
        //apic::disableIrq(0x22);
    } else {
        LOG_STARTUP("APIC not supported, skipping. (Threading will not be supported)");
    }
    mod++;

    for (multiboot_uint32_t i = 1 ; i < multiboot->mods_count ; i++) {
        printf("loading module %d at 0x%X\n", i, mod->mod_start);
        u8* c = (u8*) mod->mod_start;
        u8* data = new u8[mod->mod_end - mod->mod_start];
        memcpy(data, c, mod->mod_end - mod->mod_start);

        elf::elf* e = new elf::elf(data, true);
        if (e == nullptr) {
            printf("Failed allocating elf\n");
            while (1);
        }

        if (e->link_as_kernel_module(*symbolMap) != 0 && 0) {
            printf("failed linking elf\n");
        } else {
            void (*module_load)(bindings::Binding*);
            e->get_symbol_value("module_load", (u32*) &module_load);
            module_load(bindings::root);
        }

        mod++;
    }

    cpu_main();
}
