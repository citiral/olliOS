#include <stdio.h>
#include <types.h>
#include "libc/libc.h"

extern "C" void sysint(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi, u32 edi, u32 ebp);


extern "C" void main(int argc, char **argv) __attribute__((section(".start")));
extern "C" void main(int argc, char **argv)
{
    //test();
    sysint(50, 1, 2, 3, 4, 5, 6);
}
