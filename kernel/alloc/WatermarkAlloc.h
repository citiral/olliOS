//
// Created by citiral on 9/23/16.
//

#ifndef OLLIOS_GIT_WATERMARKALLOC_H
#define OLLIOS_GIT_WATERMARKALLOC_H

#include <stddef.h>

class WatermarkAlloc {
public:
    WatermarkAlloc();

    void init(void* start, size_t length);

    void* malloc(size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);
    void* calloc(size_t num, size_t size);

//private:
    void* start;
    size_t remaining;
};


void allocInit();

#endif //OLLIOS_GIT_WATERMARKALLOC_H
