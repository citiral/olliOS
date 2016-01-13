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

PageDirectory directory __PAGE_ALIGNED;
PageTable table1 __PAGE_ALIGNED;
PageTable table2 __PAGE_ALIGNED;
PageTable table3 __PAGE_ALIGNED;

int printa(int b)
{
	printf("a: %d", b);
}

extern "C" void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	initialize_tss(0x10, 0x28);
	PicInit();
}

extern "C" void main() {
	initCpu();
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

	printf("pd = %X\n", &directory);
	printf("val1 = %X\n", directory.get((void*)0xC0000000).value);
	printf("t1 = %X\n", &table1);

	for (size_t i = 0 ; i < 6 ; i++)
	{
		printf("t1%d = %X\n", i, *((u32*)(char*)table1.entries + i));
	}

	printf("t1%d = %X\n",1, *((u32*)&table1.entries + 0));
	printf("t1%d = %X\n",2, *((u32*)&table1.entries + 1));
	printf("t1%d = %X\n",3, *((u32*)&table2.entries + 2));
	printf("t1%d = %X\n",4, *((u32*)&table2.entries + 3));
	printf("t1%d = %X\n",5, *((u32*)&table2.entries + 4));
	printf("t1%d = %X\n",6, *((u32*)&table2.entries + 5));

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

	//directory.use();

	PRINT_INIT("...paging initialized.");
	// first we properly initialize paging
	//    1: we identity map

	//setup p

	PRINT_INIT("Hello world!");
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

		__asm__ volatile("hlt");
	}
}
