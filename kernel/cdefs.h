#ifndef __CDEFS_H
#define __CDEFS_H

#include "stdio.h"

#define __PAGE_ALIGNED __attribute__((aligned(0x1000)))
#define __PACKED __attribute__ ((__packed__))
#define BOCHS_BREAKPOINT asm __volatile__ ("xchgw %bx, %bx");

#define PRINT_INIT(X, ...) (printf("[init] " X "\n", ##__VA_ARGS__))
#define PRINT_UNIMPLEMENTED (printf("[UNIMPLEMENTED] %s:%s", __FILE__, __LINE__))

#define UNIMPLEMENTED(FUNC, R) FUNC { PRINT_UNIMPLEMENTED; return R; }

#define SIZEOF_GB 0x40000000
#define SIZEOF_MB 0x100000
#define SIZEOF_KB 0x400

#endif /* end of include guard: __CDEFS_H */
