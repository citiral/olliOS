//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "memory/alloc.h"
#include "stdio.h"
#include "cpu/cpu.h"

void* calloc(size_t num, size_t size) {
#ifdef HEAP_SMASH_DETECTION
	unsigned char* data = (unsigned char*) kernelAllocator.calloc(num, size + 12);
	return data + 8;
#else
	return kernelAllocator.calloc(num, size);
#endif
}
