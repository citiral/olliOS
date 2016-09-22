#include "types.h"
#include "vga.h"
#include "gdt.h"
#include "interrupt.h"
#include "pic.h"
#include "singleton.h"
#include "devicemanager.h"
#include "io.h"
#include "string.h"
#include "inputformatter.h"
#include "keyboard.h"
#include "stdio.h"
#include "paging.h"
#include "pagealloc.h"



int printa(int b)
{
	printf("a: %d", b);
}

extern "C" void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	initialize_tss(0x10, 0x28);
	PicInit();
    PageInit();
}

extern "C" void main() {
	initCpu();

	PRINT_INIT("Welcome to OlliOS!");

	InputFormatter fmt;
	while (true) {
		VirtualKeyEvent input[10];
		size_t read = keyboardDriver.read(input, 10);

		for (size_t i = 0 ; i < read ; i += sizeof(VirtualKeyEvent))
		{
			fmt.handleVirtualKeyEvent(input[i]);
		}

		__asm__ volatile("hlt");
	}
}
