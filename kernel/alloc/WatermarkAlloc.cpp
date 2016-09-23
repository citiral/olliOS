//
// Created by citiral on 9/23/16.
//

#include "WatermarkAlloc.h"
#include "linker.h"
#include <stdio.h>
#include <string.h>

WatermarkAlloc::WatermarkAlloc() {

}

void WatermarkAlloc::init(void* start, size_t length) {
    this->start = start;
    remaining = length;
}

void* WatermarkAlloc::malloc(size_t size) {
    // allocate size bytes + 1 size_t to remember the length
    if (remaining < size + sizeof(size_t))
        printf("OUT OF MEMORY");

    start += size + sizeof(size_t);
    remaining -= size + sizeof(size_t);

    *(size_t*)(start - size -  sizeof(size_t)) = size;
    return start - size;
}

void* WatermarkAlloc::realloc(void* ptr, size_t size) {
    size_t originalsize = *(size_t*)(ptr - sizeof(size_t));

    void* next = malloc(size);

    if (originalsize < size)
        memcpy(next, ptr, originalsize);
    else
        memcpy(next, ptr, size);

    free(ptr);

    return next;
}

void WatermarkAlloc::free(void* ptr) {
    // eh, fuck it
}

void* WatermarkAlloc::calloc(size_t num, size_t size) {
    void* ptr = malloc(num * size);
    memset(ptr, 0, num*size);
    return ptr;
}