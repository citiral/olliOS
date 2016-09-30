//
// Created by citiral on 9/23/16.
//

#include "alloc.h"

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_LINEAR
LinearAlloc kernelAllocator = LinearAlloc();
#elif __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
BucketAlloc kernelAllocator = BucketAlloc();
#endif
