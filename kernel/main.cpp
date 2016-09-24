#include "types.h"
#include "gdt.h"
#include "interrupt.h"
#include "pic.h"
#include "io.h"
#include "string.h"
#include "inputformatter.h"
#include "keyboard.h"
#include "stdio.h"
#include "paging.h"
#include "alloc.h"
#include "linker.h"
#include "alloc.h"
#include <stdlib.h>

int printa(int b)
{
	printf("a: %d", b);
}

void initCpu() {
	GdtCreateFlat();
	GdtFlush();
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
	initialize_tss(0x10, 0x28);
	PicInit();
    PageInit();
}

void initKernel() {
    // initialize the allocator to use memory from the end of the kernel up to 0xFFFFFFFF (which is 1gb - size of the kernel)
    kernelAllocator.init(KERNEL_END, 0xFFFFFFFF - (size_t)KERNEL_END);
}

void alloctest(size_t size, int count) {
    char** array = (char**)malloc(count * sizeof(char*));
    printf("arr is %x, size is %d alloced %d\n", array, *(((size_t*)array) - 2) ^ 0x80000000, count * sizeof(char*));

    for (int i = 0; i < count; i++) {
        array[i] = (char*)malloc(size);
        //for (int k = 0 ; k < size ; k++)
        //    array[i][k] = 0;
    }
    for (int i = 0; i < count; i++) {
        //printf("&&");
        free(array[i]);
    }
    //printf("yy");
    free(array);
}

void alloctest2(size_t size, int count) {
    char** array = (char**)malloc(count * sizeof(char*));
    printf("arr size is %d alloced %d\n", *(((size_t*)array) - 2) ^ 0x80000000, count * sizeof(char*));
    for (int i = 0; i < count; i++) {
        array[i] = (char*)malloc(size);
        //printf("LOOL, 0: %X, 1: %X\n", ((size_t*)0xC0240170)[0], ((size_t*)0xC0240170)[1]);
        //for (int k = 0 ; k < size ; k++)
        //    array[i][k] = 0;
    }
    for (int i = 0; i < count; i++) {
        //printf("&&");
        free(array[i]);
    }
    //printf("yy");
    free(array);
}

extern "C" void main() {
    // init CPU related stuff
	initCpu();
    // init kernel related stuff
    initKernel();

	PRINT_INIT("Welcome to OlliOS!");

    //kernelAllocator.printStatistics();
    //kernelAllocator.printStatistics();
    //kernelAllocator.printStatistics();
    //printf("lsize: %X\n", *((size_t*)kernelAllocator.buckets[29]));
    //alloctest(1, 1);
    printf("lsize: %X\n", *((size_t*)kernelAllocator.buckets[29]));
    printf("w");
    alloctest(4, 2);

    alloctest(40, 2);
    printf("a");
    alloctest2(80, 3);
    printf("b");
    //alloctest(45678, 1);
    //alloctest(1, 1);
    printf("b");
    //alloctest(910, 1);

    kernelAllocator.merge();
    printf("lsize: %X\n", *((size_t*)kernelAllocator.buckets[29]));
    //kernelAllocator.printStatistics();

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
