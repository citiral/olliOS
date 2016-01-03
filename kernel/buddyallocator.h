#ifndef __BUDDY__ALLOCATOR_H
#define __BUDDY__ALLOCATOR_H

#include "singleton.h"
#include <stddef.h>

//TODO everything

class BuddyAllocator {
public:
    BuddyAllocator(size_t blocksize, void* start, size_t size);
    ~BuddyAllocator();

private:
    size_t _blocksize;
    size_t _size;
    void* _start;

};

#endif /* end of include guard: __BUDDY__ALLOCATOR_H */
