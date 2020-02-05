#include "libc.h"
#include <types.h>

extern "C" void sysint(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi, u32 edi, u32 ebp);

extern "C" void test(void)
{
    sysint(10, 9, 8, 7, 6, 5, 4);
}