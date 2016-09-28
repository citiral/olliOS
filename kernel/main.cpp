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

struct Foo {
	Foo() {
		printf("foo has been created!\n");
	}

	~Foo() {
		printf("RIP foo.\n");
	}
};

void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	initialize_tss(0x10, 0x28);
	PicInit();
    PageInit();
}

void initKernel() {
    // initialize the allocator to use memory from the end of the kernel up to 0xFFFFFFFF (which is 1gb - size of the kernel)
    kernelAllocator.init(KERNEL_END, 0xCFFFFFFF - (size_t)KERNEL_END);

    // initialize the ata driver, this discovers the ata devices and registers them elsewhere in the kernel
    // ataDriver.initialize();

    // and register the default vga and and keyboard driver
    // deviceManager.addDevice(&vgaDriver);
    // deviceManager.addDevice(new KeyboardDriver());
}

extern "C" void main(Multiboot_info* multiboot) {
    // init CPU related stuff
	initCpu();
    // init kernel related stuff
    initKernel();

    printf("Multiboot flags: %X\n", *(size_t*)multiboot);
    printf("Multiboot flags: %X\n", multiboot->flags);

    if ((multiboot->flags & (1 << 6)) == 0) {
        printf("No MMAP specified by multiboot.. :(");
    } else {
        /*for (int i = 0; i < multiboot->mmap_length; i++) {
            printf("Found memory of type %d, starting from %X with length %X\n", multiboot->mmap_addr[i].type,
                   multiboot->mmap_addr[i].base_addr_lower, multiboot->mmap_addr[i].length_lower);
        }*/
    }

	PRINT_INIT("Welcome to OlliOS!");

    //InputFormatter fmt;

    //kernelAllocator.printStatistics();



    /*for (int i = 0 ; i < 20 ; i++) {
        printf("alloccing [%d]\n", i);
        free(malloc(i * 2 + 1 * sizeof(int)));
        kernelAllocator.printStatistics();
    }*/

    /*std::vector<char> beep;
    printf("\n");
    for (int i = 0 ; i < 100 ; i++) {
        printf("a");
        beep.push_back('a');
    }*/

    /*while (true) {
		VirtualKeyEvent input[10];
		size_t read = deviceManager.getDevice(DeviceType::Keyboard, 0)->read(input, 10);

		for (size_t i = 0 ; i < read ; i += sizeof(VirtualKeyEvent))
		{
			fmt.handleVirtualKeyEvent(input[i]);
		}

		__asm__ volatile("hlt");
	}*/
}
