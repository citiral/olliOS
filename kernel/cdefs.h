#ifndef __CDEFS_H
#define __CDEFS_H

#include "stdio.h"

#define __PAGE_ALIGNED __attribute__((aligned(0x1000)))
#define __PACKED __attribute__ ((__packed__))

#define PRINT_INIT(X, ...) (printf("[init] " X "\n", ##__VA_ARGS__))

#endif /* end of include guard: __CDEFS_H */
