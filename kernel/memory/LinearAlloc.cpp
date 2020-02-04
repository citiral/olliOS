//
// Created by citiral on 9/23/16.
//

#include "LinearAlloc.h"
#include "linker.h"
#include "cdefs.h"
#include <stdio.h>
#include <string.h>

LinearAlloc::LinearAlloc() {

}

void LinearAlloc::init(void* start, size_t length) {
    this->start = static_cast<char*>(start);
    remaining = length;
}

void* LinearAlloc::malloc(size_t size) {
    // allocate size bytes + 1 size_t to remember the length
    if (remaining < size + sizeof(size_t)) {
        printf("OUT OF MEMORY");
        return nullptr;
    }

    start += size + sizeof(size_t);
    remaining -= size + sizeof(size_t);

    *(size_t*)(start - size -  sizeof(size_t)) = size;
    return start - size;
}

void* LinearAlloc::realloc(void* ptr, size_t size) {
    size_t originalsize = *(size_t*)(static_cast<char*>(ptr) - sizeof(size_t));

    void* next = malloc(size);

    if (originalsize < size)
        memcpy(next, ptr, originalsize);
    else
        memcpy(next, ptr, size);

    free(ptr);

    return next;
}

void LinearAlloc::free(void* ptr) {
	UNUSED(ptr);
    // eh, fuck it
}

void* LinearAlloc::calloc(size_t num, size_t size) {
    void* ptr = malloc(num * size);
    memset(ptr, 0, num*size);
    return ptr;
}