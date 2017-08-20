//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "alloc.h"

void* calloc(size_t num, size_t size) {
	return kernelAllocator.calloc(num, size);
}
