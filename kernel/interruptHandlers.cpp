#include "interruptHandlers.h"
#include "interrupt.h"
#include "devicemanager.h"
#include "devices/vga.h"
#include "devices/keyboard.h"
#include "io.h"
#include "pic.h"
#include "ata/ata.h"
#include "stdio.h"
#include "cpu.h"
#include "cdefs.h"

void intHandlerUndefined(u32 interrupt) {
	printf("Undefined interrupt has been thrown: 0x%x\n", interrupt);

    // if the interrupt was thrown by an IRQ, end the interrupt
    if (interrupt >= 0x20 && interrupt <= 0x2F)
        endInterrupt(interrupt - 0x20);

    /*while (true) {
        __asm volatile("cli");
        __asm volatile("hlt");
    }*/

}

/*void intHandlerKeyboard(u32 interrupt) {
    // get the keyboard button that has been pressed
	u8 data = inb(0x60);

    // send it to the keyboarddriver
	((KeyboardDriver*) deviceManager.getDevice(DeviceType::Keyboard, 0))->write(data);

    // and end the interrupt
	endInterrupt(interrupt - 0x20);
}*/

void intHandlerAta(u32 interrupt) {
	UNUSED(interrupt);
    ataDriver.notifyInterrupt();
    endInterrupt(0x20);
}

void intHandlerGeneralProtectionViolation(u32 interrupt)
{
	UNUSED(interrupt);
	CPU::panic("General Protection Violation occured");
}

void intHandlerPageFault(u32 interrupt)
{
	UNUSED(interrupt);
	CPU::panic("Page Fault occured");
}