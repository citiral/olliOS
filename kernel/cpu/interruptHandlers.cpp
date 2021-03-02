#include "interrupt.h"
#include "interruptHandlers.h"
#include "cdefs.h"
#include "pic.h"
#include "threading/thread.h"
#include "process.h"

extern "C" void intHandlerGeneralProtectionViolation()
{
	u16 CS = 0;
	u16 DS = 0;
	u16 SS = 0;
	u32 ESP = 0;

	__asm__ volatile("mov %%esp, %0;": "=r"(ESP));
	__asm__ volatile("mov %%cs, %0;": "=r"(CS));
	__asm__ volatile("mov %%Ds, %0;": "=r"(DS));
	__asm__ volatile("mov %%Ss, %0;": "=r"(SS));

	end_interrupt(INT_GENERAL_PROTECTION_VIOLATION);

	printf("ESP\tCS\tDS\tSS\n%X\t%X\t%X\t%x\n", ESP, CS, DS, SS);
	CPU::panic("General Protection Violation occured");
}

extern "C" void intHandlerPageFault()
{
	u32 cr2;

	// Read the CR2 register to get the address of the pagefault
	asm volatile (
		"mov %%cr2, %0\n"
		: "=r" (cr2)
	);

	printf("Page fault: 0x%.8X\n", cr2);

	if (threading::is_current_core_in_thread()) {
		end_interrupt(INT_PAGE_FAULT);
		threading::currentThread()->process->exit(11);
	} else {
		CPU::panic("Page Fault occured");
	}
}