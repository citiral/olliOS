//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "alloc.h"

void* realloc(void* ptr, size_t size) {
    kernelAllocator.realloc(ptr, size);
}
