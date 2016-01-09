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
}

PageDirectory directory __PAGE_ALIGNED;
PageTable table1 __PAGE_ALIGNED;
PageTable table2 __PAGE_ALIGNED;
PageTable table3 __PAGE_ALIGNED;

extern "C" void main() {
	PRINT_INIT("initializing paging...");

	directory.clear();

	table1.mapAll((void*)0x00000000);
	table2.mapAll((void*)0x00400000);
	table3.mapAll((void*)0x00800000);

	directory.get((void*)0x00000000).setAddress(((char*)&table1) - 0xC0000000);
	directory.get((void*)0xC0000000).setAddress(((char*)&table1) - 0xC0000000);
	directory.get((void*)0xC0400000).setAddress(((char*)&table2) - 0xC0000000);
	directory.get((void*)0xC0800000).setAddress(((char*)&table3) - 0xC0000000);

	directory.get((void*)0x00000000).enableFlag(PFLAG_PRESENT | PFLAG_RW);
	directory.get((void*)0xC0000000).enableFlag(PFLAG_PRESENT | PFLAG_RW);
	directory.get((void*)0xC0400000).enableFlag(PFLAG_PRESENT | PFLAG_RW);
	directory.get((void*)0xC0800000).enableFlag(PFLAG_PRESENT | PFLAG_RW);

	/*directory.get((void*)0x00000000).setAddress((void*)0x00000000);
	directory.get((void*)0xC0000000).setAddress((void*)0x00000000);
	directory.get((void*)0xC0400000).setAddress((void*)0x00400000);
	directory.get((void*)0xC0800000).setAddress((void*)0x00800000);

	directory.get((void*)0x00000000).enableFlag(PFLAG_PRESENT | PFLAG_RW | PFLAG_LARGEPAGE);
	directory.get((void*)0xC0000000).enableFlag(PFLAG_PRESENT | PFLAG_RW | PFLAG_LARGEPAGE);
	directory.get((void*)0xC0400000).enableFlag(PFLAG_PRESENT | PFLAG_RW | PFLAG_LARGEPAGE);
	directory.get((void*)0xC0800000).enableFlag(PFLAG_PRESENT | PFLAG_RW | PFLAG_LARGEPAGE);*/


	/*directory.get((void*)0x00000000).setAddress(((char*)&table1) - 0xC0000000);
	directory.get((void*)0xC0000000).setAddress(((char*)&table1) - 0xC0000000);
	directory.get((void*)0xC0400000).setAddress(((char*)&table2) - 0xC0000000);
	directory.get((void*)0xC0000000).enableFlag(PFLAG_PRESENT);
	directory.get((void*)0xC0400000).enableFlag(PFLAG_PRESENT);
	directory.get((void*)0xC0000000).enableFlag(PFLAG_RW);
	directory.get((void*)0xC0400000).enableFlag(PFLAG_RW);

	printf("table 1 is %u\n", directory.get((void*)0).value);
	printf("table 2 is %u\n", directory.get((void*)0).value);*/

	directory.use();

	PRINT_INIT("...paging initialized.");
	// first we properly initialize paging
	//    1: we identity map

	//setup p

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
