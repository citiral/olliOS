#include "buddyallocator.h"

void* operator new(size_t size) {
    return (void*)0;
}

void operator delete(void* ptr) {

}

BuddyAllocator::BuddyAllocator(size_t blocksize, void* start, size_t size):
    _blocksize(blocksize), _start(start), _size(size)
{

}

BuddyAllocator::~BuddyAllocator()
{

}
