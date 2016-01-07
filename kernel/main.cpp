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
#include "pagealloc.h"

void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	printf("Created GDT.\n");
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	printf("Created IDT.\n");
	initialize_tss(0x10, 0x28);
	printf("Created TSS.\n");
	THROW_INTERRUPT(250);
}

extern "C" void main() {
	printf("Hello world!\n");
	initCpu();
	PicInit();
	printf("Welcome to OlliOS!\n");

	PageEntry entry;
	initializeEntry(entry);


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
