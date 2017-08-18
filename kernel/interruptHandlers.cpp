#include "interruptHandlers.h"
#include "interrupt.h"
#include "devicemanager.h"
#include "streams/vga.h"
#include "io.h"
#include "pic.h"
#include "ata/ata.h"
#include "stdio.h"

void dumpStack(unsigned int MaxFrames)
{
	unsigned int * ebp = &MaxFrames - 2;
	printf("Stack trace:\n");
	for(unsigned int frame = 0; frame < MaxFrames; ++frame)
	{
		unsigned int eip = ebp[1];
		if(eip == 0)
			// No caller on stack
			break;
		// Unwind to previous stack frame
		ebp = reinterpret_cast<unsigned int *>(ebp[0]);
		unsigned int * arguments = &ebp[2];
		printf("  0x%X     \n", eip);
	}
}

void halt()
{
	dumpStack(10);
	printf("Halting\n");
	
	while (true)
	{
		__asm__ volatile("cli");
		__asm__ volatile("hlt");
	}
}

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

void intHandlerKeyboard(u32 interrupt) {
    // get the keyboard button that has been pressed
	u8 data = inb(0x60);

    // send it to the keyboarddriver
    deviceManager.getDevice(DeviceType::Keyboard, 0)->write(data);

    // and end the interrupt
	endInterrupt(interrupt - 0x20);
}

void intHandlerAta(u32 interrupt) {
    ataDriver.notifyInterrupt();
    endInterrupt(0x20);
}

void intHandlerGeneralProtectionViolation(u32 interrupt)
{
	printf("General Protection Violation occured\n");
	halt();
}

void intHandlerPageFault(u32 interrupt)
{
	printf("Page Fault occured\n");
	halt();
}