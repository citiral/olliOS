//
// Created by citiral on 9/23/16.
//

#ifndef OLLIOS_GIT_ALLOC_H
#define OLLIOS_GIT_ALLOC_H

#include "WatermarkAlloc.h"

#define USE_WATERMARK_ALLOCATOR

#ifdef USE_WATERMARK_ALLOCATOR
extern WatermarkAlloc kernelAllocator;
#endif

#endif //OLLIOS_GIT_ALLOC_H
