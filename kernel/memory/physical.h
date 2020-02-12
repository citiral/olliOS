#pragma once

#include "types.h"
#include <string.h>

#define DEVICE_MAX_MEMORY 0xFFFFFFFF

namespace memory {
    class PhysicalMemoryManager {
    public:
        void init();
        void registerAvailableMemory(void* start, size_t length);
        void reservePhysicalMemory(void* start, size_t length);
        void* allocatePhysicalMemory();
        // Allocate but the returned address will contain a 0 where the reverseMask has a 1
        // This can be used to allocate something on a specific boundary
        void* allocatePhysicalMemory(size_t reverseMask);
        void freePhysicalMemory(void* memory);
        size_t countFreePhysicalMemory();


    private:
        // We have a bitmap where each bit represents a free page. Each byte can represent 8*4KB
        u8 _bitmap[DEVICE_MAX_MEMORY / (0x8000)];
    };

    extern PhysicalMemoryManager physicalMemoryManager;
}