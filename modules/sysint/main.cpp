
#include "bindings.h"
#include "cpu/interrupt.h"
#include "threading/thread.h"
#include "process.h"

using namespace bindings;

extern "C" void sysint_handler(void);

#define SYSINT_OPEN 1
#define SYSINT_CLOSE 2
#define SYSINT_WRITE 3
#define SYSINT_READ 4


extern "C" i32 sysint_handler_c(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi, u32 edi, u32 ebp) {
    //printf("sysint: %d %d %d %d %d %d\n", eax, ebx, ecx, edx, esi, edi, ebp);
    if (eax == SYSINT_OPEN) {
        return threading::currentThread()->process()->open(reinterpret_cast<const char*>(ebx), reinterpret_cast<i32&>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_CLOSE) {
        return threading::currentThread()->process()->close(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_WRITE) {
        return threading::currentThread()->process()->write(reinterpret_cast<i32&>(ebx), reinterpret_cast<char *>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_READ) {
        return threading::currentThread()->process()->read(reinterpret_cast<i32&>(ebx), reinterpret_cast<char *>(ecx), reinterpret_cast<i32&>(edx));
    }

    return -1;
}

extern "C" void module_load(Binding* root)
{
    idt.getEntry(0x80).setOffset((u32) sysint_handler);
}