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
#include <stdlib.h>

int printa(int b)
{
	printf("a: %d", b);
}

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
    kernelAllocator.init(KERNEL_END, 0xFFFFFFFF - (size_t)KERNEL_END);
}

extern "C" void main() {
    // init CPU related stuff
	initCpu();
    // init kernel related stuff
    initKernel();

	PRINT_INIT("Welcome to OlliOS!");

    InputFormatter fmt;

	while (true) {
		VirtualKeyEvent input[10];
		size_t read = keyboardDriver.read(input, 10);

		for (size_t i = 0 ; i < read ; i += sizeof(VirtualKeyEvent))
		{
			fmt.handleVirtualKeyEvent(input[i]);
		}

		__asm__ volatile("hlt");
	}
}
