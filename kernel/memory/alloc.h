//
// Created by citiral on 9/23/16.
//

#ifndef OLLIOS_GIT_ALLOC_H
#define OLLIOS_GIT_ALLOC_H

#include "BucketAlloc.h"
#include "LinearAlloc.h"
#include "options.h"

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_LINEAR
extern LinearAlloc kernelAllocator;
#elif __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
extern BucketAlloc kernelAllocator;
#endif

#endif //OLLIOS_GIT_ALLOC_H
