#include "interruptHandlers.h"
#include "keyboard.h"
#include "vga.h"
#include "io.h"
#include "pic.h"

void intHandlerUndefined(u32 interrupt) {
	printf("Undefined interrupt has been thrown: %d\n", interrupt);

    // if the interrupt was thrown by an IRQ, end the interrupt
    if (interrupt >= 0x20 && interrupt <= 0x2F)
        endInterrupt(interrupt - 0x20);

    while (true) {
        __asm volatile("cli");
        __asm volatile("hlt");
    }

}

void intHandlerKeyboard(u32 interrupt) {
    // get the keyboard button that has been pressed
	u8 data = inb(0x60);

    // send it to the keyboarddriver
    keyboardDriver.write(&data, 1);

    // and end the interrupt
	endInterrupt(interrupt - 0x20);
}
