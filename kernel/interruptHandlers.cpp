#include "interruptHandlers.h"
#include "keyboard.h"
#include "vga.h"
#include "io.h"
#include "pic.h"

void intHandlerUndefined(u32 interrupt) {
	printf("Undefined interrupt has been thrown: %d\n", interrupt);
	endInterrupt(interrupt);
}

void intHandlerKeyboard(u32 interrupt) {
    // get the keyboard button that has been pressed
	u8 data = inb(0x60);

    // send it to the keyboarddriver
    keyboardDriver.write(&data, 1);

    // and end the interrupt
	endInterrupt(interrupt);
}
