
#include "bindings.h"
#include "cpu/interrupt.h"

using namespace bindings;

extern "C" void sysint_handler(void);

extern "C" void sysint_handler_c(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi, u32 edi, u32 ebp) {
    printf("sysint %d %d %d %d %d %d %d\n", eax, ebx, ecx, edx, esi, edi, ebp);
}

extern "C" void module_load(Binding* root)
{
    idt.getEntry(0x80).setOffset((u32) sysint_handler);
}