#include "types.h"
#include "vga.h"
#include "gdt.h"
#include "interrupt.h"
#include "pic.h"
#include "singleton.h"
#include "devicemanager.h"
#include "io.h"
#include "string.h"
#include "inputformatter.h"
#include "keyboard.h"
#include "stdio.h"
#include "paging.h"
#include "pagealloc.h"

void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	PRINT_INIT("Created GDT.");
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	PRINT_INIT("Created IDT.");
	initialize_tss(0x10, 0x28);
	PRINT_INIT("Created TSS.");
	THROW_INTERRUPT(250);
}

extern "C" void main() {
	// first we properly initialize paging
	//    1: we identity map 

	PRINT_INIT("Hello world!");
	initCpu();
	PicInit();
	



	//pageAllocatorInitialize((void*)0x01000000, 1*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2 * 0x1000);
	PRINT_INIT("Welcome to OlliOS!");

	InputFormatter fmt;

	while (true) {
		VirtualKeyEvent input[10];
		size_t read = keyboardDriver.read(input, 10);

		for (size_t i = 0 ; i < read ; i += sizeof(VirtualKeyEvent))
		{
			fmt.handleVirtualKeyEvent(input[i]);
		}
		pagingEnablePSE();
		__asm__ volatile("hlt");
	}
}
