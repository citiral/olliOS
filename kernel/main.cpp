#include "types.h"
#include "vga.h"
#include "gdt.h"
#include "interrupt.h"
#include "pic.h"
#include "singleton.h"
#include "devicemanager.h"
#include "io.h"
#include "string.h"

void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	vgaDriver.write("Created GDT.\n");
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	vgaDriver.write("Created IDT.\n");
	initialize_tss(0x10, 0x28);
	vgaDriver.write("Created TSS.\n");
	THROW_INTERRUPT(250);
}

extern "C" void main() {
	vgaDriver.write("Hello world!\n");
	initCpu();
	PicInit();
	outb(0x64, 0x60);
	outb(0x60, 0b00000001);
	vgaDriver.write("Welcome to OlliOS!\n");

	while (true) {
		__asm__ volatile("hlt");
	}
}
