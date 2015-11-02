#include "types.h"
#include "vga.h"
#include "gdt.h"
#include "interrupt.h"

void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	VgaWriteChars("Created GDT.\n");
	IdtcreateEmpty();
	IdtFlush();
	VgaWriteChars("Created IDT.\n");
	initialize_tss(0x10, 0x28);
	VgaWriteChars("Created TSS.\n");
}

extern "C" void main() {
	VgaClear();
	initCpu();
	VgaWriteChars("Welcome to OlliOS!\n");
}