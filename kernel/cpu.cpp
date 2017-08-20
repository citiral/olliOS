#include "cpu.h"
#include "stdio.h"

void CPU::dumpstack(unsigned int MaxFrames)
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
		//unsigned int * arguments = &ebp[2];
		printf("  0x%X     \n", eip);
	}
}

void CPU::panic(const char* msg)
{
	printf("Kernel Panic!!!\n");
	printf("%s\n", msg);
	dumpstack(10);
	printf("Halting\n");
	halt();
}

void CPU::panic()
{
	printf("Kernel Panic!!!\n");
	dumpstack(10);
	printf("Halting\n");
	halt();
}

void CPU::halt()
{
	while (true)
	{
		__asm__ volatile("cli");
		__asm__ volatile("hlt");
	}
}