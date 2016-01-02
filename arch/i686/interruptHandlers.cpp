#include "interruptHandlers.h"
#include "vga.h"
#include "io.h"
#include "pic.h"

void intHandlerUndefined(u32 interrupt) {
	UNUSED(interrupt);
	VgaWriteChars("Undefined interrupt has been thrown: [TODO WRITE INTERRUPT]\n");
}

void intHandlerKeyboard(u32 interrupt) {
	UNUSED(interrupt);
	VgaWriteChars("Keyboard interrupt has been thrown.\n");
	u8 scan = inb(0x60);
	endInterrupt(1);
}