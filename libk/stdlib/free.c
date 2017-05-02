//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "alloc.h"

void free(void* ptr) {
    kernelAllocator.free(ptr);
}
