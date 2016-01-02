#pragma once

#include "singleton.h"
#include <stddef.h>

class BuddyAllocator {
public:
    BuddyAllocator(size_t blocksize, void* start, size_t size);
    ~BuddyAllocator();

private:
    size_t _blocksize;
    size_t _size;
    void* _start;
    
};
