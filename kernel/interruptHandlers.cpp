#include "interrupt.h"
#include "interruptHandlers.h"
#include "devices/ata/ata.h"
#include "pic.h"

void intHandlerUndefined(u32 interrupt)
{
	LOG_ERROR("Unhandled interrupt %d", interrupt);
	end_interrupt(interrupt);
}

void intHandlerWakeup(u32 interrupt)
{
	UNUSED(interrupt);
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
