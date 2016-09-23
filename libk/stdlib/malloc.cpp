//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "alloc.h"

void* malloc(size_t size) {
    return kernelAllocator.malloc(size);
}
