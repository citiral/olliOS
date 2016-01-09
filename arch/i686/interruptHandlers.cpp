#include "interruptHandlers.h"
#include "keyboard.h"
#include "vga.h"
#include "io.h"
#include "pic.h"

void intHandlerUndefined(u32 interrupt) {
	printf("Undefined interrupt has been thrown: %d\n", interrupt);
}

void intHandlerKeyboard(u32 interrupt) {
	//vgaDriver.write("Keyboard interrupt has been thrown.\n");
	u8 data = inb(0x60);

	keyboardDriver.write(&data, 1);
	endInterrupt(1);
}
