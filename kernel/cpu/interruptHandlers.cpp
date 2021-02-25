#include "interrupt.h"
#include "interruptHandlers.h"
#include "cdefs.h"
#include "pic.h"
#include "threading/thread.h"
#include "process.h"

void intHandlerUndefined(u32 interrupt)
{
	//LOG_ERROR("Unhandled interrupt %d", interrupt);
	end_interrupt(interrupt);
}

void intHandlerWakeup(u32 interrupt)
{
	UNUSED(interrupt);
}

void intHandlerGeneralProtectionViolation(u32 interrupt)
{
	UNUSED(interrupt);
	CPU::panic("General Protection Violation occured");
}

void intHandlerPageFault(u32 interrupt)
{
	UNUSED(interrupt);
	u32 cr2;

	// Read the CR2 register to get the address of the pagefault
	asm volatile (
		"mov %%cr2, %0\n"
		: "=r" (cr2)
	);

	printf("Page fault: 0x%.8X\n", cr2);

	if (threading::is_current_core_in_thread()) {
		threading::currentThread()->process->exit(11);
	} else {
		CPU::panic("Page Fault occured");
	}
}

void intHandlerSpurious(u32 interrupt)
{
    UNUSED(interrupt);
    LOG_INFO("Spurious interrupt happened.");
}
