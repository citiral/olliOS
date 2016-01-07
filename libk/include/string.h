#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*ptr, int value, size_t num);
size_t strlen(const char* string);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: __STRING_H */
