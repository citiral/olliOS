#include "cpuid.h"

cpuid_field cpuid(int field) {
    cpuid_field output;

    __asm__ __volatile__ (
        "cpuid"
        : "=b"(output.ebx), "=c"(output.ecx), "=d"(output.edx)
        : "a"(field)
    );

    return output;
}