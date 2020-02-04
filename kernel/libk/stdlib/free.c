//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "memory/alloc.h"
#include "stdio.h"
#include "cpu/cpu.h"

void free(void* ptr) {
#ifdef HEAP_SMASH_DETECTION

    unsigned char* data = (unsigned char*) ptr;
    size_t* pre = (size_t*) (data - 8);
    size_t* post = (size_t*) (data + pre[0]);

    if (pre[1] != 0xDEADBEEF || post[0] != 0xDEADBEEF) {
        printf("FREE HEAP DAMAGED:\n");
		printf("%X %X\n", pre[1], post[0]);
        CPU::panic();
    }

    kernelAllocator.free(pre);
#else
    kernelAllocator.free(ptr);
#endif
}
