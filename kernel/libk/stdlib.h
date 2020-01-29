//
// Created by citiral on 9/23/16.
//

#ifndef OLLIOS_GIT_STDLIB_H
#define OLLIOS_GIT_STDLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);

#ifdef __cplusplus
}
#endif

#endif //OLLIOS_GIT_STDLIB_H
