//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "alloc.h"

void* malloc(size_t size) {
#ifdef HEAP_SMASH_DETECTION
    unsigned char* data = (unsigned char*) kernelAllocator.malloc(size+12);
    size_t* pre = (size_t*) (data);
    size_t* post = (size_t*) (data + 8 + size);
    pre[0] = size;
    pre[1] = 0xDEADBEEF;
    post[0] = 0xDEADBEEF;

    return data + 8;
#else
    return kernelAllocator.malloc(size);
#endif
}
