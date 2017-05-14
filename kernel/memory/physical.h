#pragma once

#include "types.h"
#include <string.h>

#define DEVICE_MAX_MEMORY 0xFFFFFFFF

class PhysicalMemoryManager {
public:
    void init();
    void registerAvailableMemory(void* start, size_t length);
    void reservePhysicalMemory(void* start, size_t length);
    void* allocatePhysicalMemory();
    void freePhysicalMemory(void* memory);

private:
    // We have a bitmap where each bit represents a free page. Each byte can represent 8*4KB
    u8 _bitmap[DEVICE_MAX_MEMORY / (0x8000)];
};

extern PhysicalMemoryManager physicalMemoryManager;