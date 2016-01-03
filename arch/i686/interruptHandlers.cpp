#include "interruptHandlers.h"
#include "keyboard.h"
#include "vga.h"
#include "io.h"
#include "pic.h"

void intHandlerUndefined(u32 interrupt) {
	vgaDriver.write("Undefined interrupt has been thrown: [TODO WRITE INTERRUPT]\n");
}

void intHandlerKeyboard(u32 interrupt) {
	vgaDriver.write("Keyboard interrupt has been thrown.\n");
	u8 data[2];
	data[0] = inb(0x60);
	data[1] = 0;
	keyboardDriver.write(data);
	endInterrupt(1);
}
