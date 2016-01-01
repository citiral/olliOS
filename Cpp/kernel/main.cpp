#include "types.h"
#include "vga.h"
#include "gdt.h"
#include "interrupt.h"
#include "pic.h"
#include "io.h"
#include <string.h>

void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	VgaWriteChars("Created GDT.\n");
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	VgaWriteChars("Created IDT.\n");
	initialize_tss(0x10, 0x28);
	VgaWriteChars("Created TSS.\n");
	THROW_INTERRUPT(250);
}

extern "C" void main() {
	memset(0, 0, 0);
	VgaClear();
	initCpu();
	PicInit();
	outb(0x64, 0x60);
	outb(0x60, 0b00000001);
	VgaWriteChars("Welcome to OlliOS!\n");

	while (true) {
		__asm__ volatile("hlt");
	}
}
