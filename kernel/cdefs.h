#ifndef __CDEFS_H
#define __CDEFS_H

#include <stdio.h>
#include <cpu.h>

#define __PAGE_ALIGNED __attribute__((aligned(0x1000)))
#define __PACKED __attribute__ ((__packed__))
#define BOCHS_BREAKPOINT asm __volatile__ ("xchgw %bx, %bx");

//#define PRINT_INIT(X, ...) (printf("[init] " X "\n", ##__VA_ARGS__))
//#define PRINT_UNIMPLEMENTED (printf("[UNIMPLEMENTED] %s:%s", __FILE__, __LINE__))

#define LOG_INFO(X, ...) (printf("[info] " X "\n", ##__VA_ARGS__))
#define LOG_DEBUG(X, ...) (printf("[debug] " X "\n", ##__VA_ARGS__))
#define LOG_ERROR(X, ...) (printf("[error] %s:%s:" X "\n", __FILE__, __LINE__, ##__VA_ARGS__))
#define LOG_STARTUP(X, ...) (printf("[startup] " X "\n", ##__VA_ARGS__))
#define LOG_UNIMPLEMENTED() (printf("[unimplemented] %s:%d\n", __FILE__, __LINE__))

#define UNIMPLEMENTED(FUNC, R) FUNC { LOG_UNIMPLEMENTED(); CPU::panic(); return R; }
#define UNUSED(X) ((void) X)

#define SIZEOF_GB 0x40000000
#define SIZEOF_MB 0x100000
#define SIZEOF_KB 0x400

#endif /* end of include guard: __CDEFS_H */
