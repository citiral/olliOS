#include "memory/physical.h"
#include <stdio.h>

PhysicalMemoryManager physicalMemoryManager;

void PhysicalMemoryManager::init() {
    // We initially set all physical memory as alocated
    memset(_bitmap, 0xFF, DEVICE_MAX_MEMORY / (0x8000));
}

void PhysicalMemoryManager::registerAvailableMemory(void* start, size_t length) {
    // set all the matching bits to 0
    for (size_t offset = 0 ; (offset + 0xFFF) < length ; offset += 0x1000) {
        size_t index = ((size_t)start + offset) / 0x1000;

        _bitmap[index/8] &= ~(0b10000000 >> (index % 8));
    }
}

void PhysicalMemoryManager::reservePhysicalMemory(void* start, size_t length) {
    // set all the matching bits to 1
    for (size_t offset = 0 ; offset < length ; offset += 0x1000) {
        size_t index = ((size_t)start + offset) / 0x1000;

        _bitmap[index/8] |= (0b10000000 >> (index % 8));
    }
}

void* PhysicalMemoryManager::allocatePhysicalMemory() {
    // TODO dit kan gelijk, efficienter
    // scan all pages
    for (size_t index = 0 ; index < DEVICE_MAX_MEMORY / 0x1000 ; index++) {
        // if the current page is set to zero
        if ((_bitmap[index/8] & (0b10000000 >> (index % 8))) == 0) {
            // set the current page to 1
            _bitmap[index/8] |= (0b10000000 >> (index % 8));
            // and return it
            return (void*)(index * 0x1000);
        }
    }
    return nullptr;
}

void* PhysicalMemoryManager::allocatePhysicalMemory(size_t reverseMask) {
	for (size_t index = 0; index < DEVICE_MAX_MEMORY / 0x1000; index++) {
		if ((((index * 0x1000) & reverseMask) == 0) && (_bitmap[index/8] & (0b10000000 >> (index % 8))) == 0) {
            // set the current page to 1
            _bitmap[index/8] |= (0b10000000 >> (index % 8));
			// and return it
            return (void*)(index * 0x1000);
        }
	}
	return nullptr;
}

void PhysicalMemoryManager::freePhysicalMemory(void* memory) {
    // get the index
    size_t index = (size_t)memory / 0x1000;
    // and set it to 0
    _bitmap[index/8] &= ~(0b10000000 >> (index % 8));
}