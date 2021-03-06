#ifndef __CDEFS_H
#define __CDEFS_H

#include "cpu/cpu.h"
#include <stdio.h>
#include <stdint.h>

#define __PAGE_ALIGNED __attribute__((aligned(0x1000)))
#define __PACKED __attribute__ ((__packed__))
#define BOCHS_BREAKPOINT asm __volatile__ ("xchgw %bx, %bx");

//#define PRINT_INIT(X, ...) (printf("[init] " X "\n", ##__VA_ARGS__))
//#define PRINT_UNIMPLEMENTED (printf("[UNIMPLEMENTED] %s:%s", __FILE__, __LINE__))

#define LOG_INFO(X, ...) (printf("[info] " X "\n", ##__VA_ARGS__))
#define LOG_DEBUG(X, ...) (printf("[debug] " X "\n", ##__VA_ARGS__))
#define LOG_ERROR(X, ...) (printf("[error@%s:%d] " X "\n", __FILE__, __LINE__, ##__VA_ARGS__))
#define LOG_STARTUP(X, ...) (printf("[startup] " X "\n", ##__VA_ARGS__))
#define LOG_UNIMPLEMENTED() (printf("[unimplemented] %s:%d\n", __FILE__, __LINE__))

#define UNIMPLEMENTED(FUNC, R) FUNC { LOG_UNIMPLEMENTED(); CPU::panic(); return R; }
#define UNUSED(X) ((void) X)

inline static uint32_t EFLAGS() {
    uint32_t flags;
     /*
      * "=rm" is safe here, because "pop" adjusts the stack before
      * it evaluates its effective address -- this is part of the
      * documented behavior of the "pop" instruction.
      */
    asm volatile("# __raw_save_flags\n\t"
              "pushfl ; pop %0"
              : "=rm" (flags)
              : /* no input */
              : "memory");

     return flags;
}

inline static bool CLI() {
    unsigned long flags;
     /*
      * "=rm" is safe here, because "pop" adjusts the stack before
      * it evaluates its effective address -- this is part of the
      * documented behavior of the "pop" instruction.
      */
    asm volatile("# __raw_save_flags\n\t"
              "pushf ; pop %0"
              : "=rm" (flags)
              : /* no input */
              : "memory");
    __asm__ volatile ("cli");
    
     return flags & 0x200;
}

inline static void STI(bool enable) {
    if (enable) {
        __asm__ volatile ("sti");
    }
}

#define SIZEOF_GB 0x40000000
#define SIZEOF_MB 0x100000
#define SIZEOF_KB 0x400

// the maximum amount of cores that can occur. This define can be used to define core-local storage.
#define MAX_CORE_COUNT 16

#endif /* end of include guard: __CDEFS_H */
