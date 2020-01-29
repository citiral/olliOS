#ifndef __STDIO_H
#define __STDIO_H

#include <stddef.h>

namespace bindings {
    class Binding;
}
typedef bindings::Binding FILE;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __cplusplus
extern "C" {
#endif

//extern FILE* stdout;

int printf(const char* format, ...);
int sprintf(char* target, const char* format, ...);
int putchar(int character);
//int fseek(FILE* stream, long int offset, int origin);

#ifdef __cplusplus
}
#endif


#endif /* end of include guard: __STDIO_H */
