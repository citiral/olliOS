#include "types.h"
#include "vga.h"
#include "gdt.h"

extern "C" void main() {
	VgaClear();
	VgaWriteChars("Welcome to OlliOS!\n");
	GdtCreateFlat();
	GdtFlush();
	VgaWriteChars("Created GDT.");
}