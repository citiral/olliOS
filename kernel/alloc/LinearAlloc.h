//
// Created by citiral on 9/23/16.
//

#ifndef OLLIOS_GIT_WATERMARKALLOC_H
#define OLLIOS_GIT_WATERMARKALLOC_H

#include <stddef.h>

class LinearAlloc {
public:
    LinearAlloc();

    void init(void* startx, size_t length);

    void* malloc(size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);
    void* calloc(size_t num, size_t size);

private:

    char* start; // char* instead of void* for C++11 compliance
    size_t remaining;
};

#endif //OLLIOS_GIT_WATERMARKALLOC_H
