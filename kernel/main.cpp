#include "types.h"
#include "gdt.h"
#include "interrupt.h"
#include "pic.h"
#include "io.h"
#include "string.h"
#include "inputformatter.h"
#include "keyboard.h"
#include "stdio.h"
#include "paging.h"
#include "alloc.h"
#include "linker.h"
#include "alloc.h"
#include "ata.h"
#include "devicemanager.h"
#include "streams/vga.h"
#include "streams/keyboard.h"
#include "multiboot.h"
#include <new>
#include <stdlib.h>

void initCpu() {
    // setup a flat segmentation structure. We're going to be using paging anyway.
	GdtCreateFlat();
	GdtFlush();

    // Then we set up the interupt tables so we can actually use interrupts (these contain function pointers to interrupt handlers)
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();

    // I forgot what this does but it has to do with interrupts.. I believe
	initialize_tss(0x10, 0x28);

    // Program the PIC so interrupts make sense (0-32 is reserved for intel, ...)
	PicInit();

    // and finally initialize paging from C since it's about time.
    PageInit();
}

extern "C" void main(multiboot_info* multiboot) {
    // init lowlevel CPU related stuff
	initCpu();

    // use the multiboot header to discover valid memory regions
    if ((multiboot->flags & (1 << 6)) == 0) {
        // if multiboot didn't pass us our usable memory areas (which it really always should) use a default.
        printf("No MMAP specified by multiboot.. :(, we'll YOLO allocate the last gb to the memory allocator");
        kernelAllocator.init((void*)KERNEL_END_VIRTUAL, 0xFFFFFFFF - (size_t)KERNEL_END_VIRTUAL);
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

            // only use the first physical gb
            if ((size_t)addr >= 0x40000000)
                continue;

            // don't let this shit overlap with the kernel itself
            if ((size_t)addr < (size_t)KERNEL_END_PHYSICAL) {
                size_t diff = (size_t)KERNEL_END_PHYSICAL - (size_t) addr;
                addr = (void*)((size_t)addr + diff);

                if (length < diff)
                    continue;

                length -= diff;
            }

            kernelAllocator.init(addr + 0xC0000000, length);
        }
    }

	PRINT_INIT("Welcome to OlliOS!");

    // initialize the ATA driveer
    ataDriver.initialize();

    // and register the default vga and and keyboard driver
    deviceManager.addDevice(&vgaDriver);
    deviceManager.addDevice(new KeyboardDriver());

    InputFormatter fmt;

    while (true) {
		VirtualKeyEvent input[10];
		int read = deviceManager.getDevice(DeviceType::Keyboard, 0)->read(input, 10);

		for (size_t i = 0 ; i < read ; i += sizeof(VirtualKeyEvent))
		{
			fmt.handleVirtualKeyEvent(input[i]);
		}

        //if (fmt.isLineReady()) {

        //}

		__asm__ volatile("hlt");
	}
}
