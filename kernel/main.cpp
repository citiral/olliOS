#include "types.h"
#include "vga.h"
#include "gdt.h"
#include "interrupt.h"
#include "pic.h"
#include "singleton.h"
#include "devicemanager.h"
#include "io.h"
#include "string.h"
#include "InputFormatter.h"
#include "keyboard.h"

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
	vgaDriver.write("Welcome to OlliOS!\n");

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
