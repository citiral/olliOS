#include "interruptHandlers.h"
#include "devices/ata/ata.h"
#include "pic.h"


/*void intHandlerKeyboard(u32 interrupt) {
    // get the keyboard button that has been pressed
	u8 data = inb(0x60);

    // send it to the keyboarddriver
	((KeyboardDriver*) deviceManager.getDevice(DeviceType::Keyboard, 0))->write(data);

    // and end the interrupt
	endInterrupt(interrupt - 0x20);
}*/

void intHandlerWakeup(u32 interrupt)
{
	LOG_INFO("WOKE");
}

void intHandlerAta(u32 interrupt) {
	UNUSED(interrupt);
    ataDriver.notifyInterrupt();
    endInterrupt(interrupt);
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

void intHandlerSpurious(u32 interrupt)
{
    UNUSED(interrupt);
    LOG_INFO("Spurious interrupt happened.");
}