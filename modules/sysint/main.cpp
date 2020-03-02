
#include "bindings.h"
#include "cpu/interrupt.h"
#include "threading/thread.h"
#include "process.h"

using namespace bindings;

extern "C" void sysint_handler(void);

#define SYSINT_OPEN   1
#define SYSINT_CLOSE  2
#define SYSINT_WRITE  3
#define SYSINT_READ   4
#define SYSINT_EXIT   5
#define SYSINT_FORK   6
#define SYSINT_GETPID 7
#define SYSINT_EXECVE 8
#define SYSINT_WAIT   9
#define SYSINT_ISATTY 10
#define SYSINT_LSEEK  11
#define SYSINT_FSTAT  12
#define SYSINT_KILL   13
#define SYSINT_LINK   14
#define SYSINT_SBRK   15
#define SYSINT_TIMES  16
#define SYSINT_UNLINK 17

const char* sysint_names[] = {
    "", "open", "close", "write", "read", "exit", "fork", "getpid", "execve", "wait", "isatty", "lseek", "fstat", "kill", "link", "sbrk", "times", "link"
};

extern "C" i32 sysint_handler_c(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi, u32 edi, u32 ebp) {
    //printf("SYS: [%s]: %x %x %x %x %x %x\n", sysint_names[eax], ebx, ecx, edx, esi, edi, ebp);

    if (eax == SYSINT_OPEN) {
        return threading::currentThread()->process->open(reinterpret_cast<const char*>(ebx), reinterpret_cast<i32&>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_CLOSE) {
        return threading::currentThread()->process->close(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_WRITE) {
        return threading::currentThread()->process->write(reinterpret_cast<i32&>(ebx), reinterpret_cast<char *>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_READ) {
        return threading::currentThread()->process->read(reinterpret_cast<i32&>(ebx), reinterpret_cast<char *>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_EXIT) {
        return threading::currentThread()->process->exit(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_FORK) {
        return threading::currentThread()->process->fork();
    } else if (eax == SYSINT_GETPID) {
        return threading::currentThread()->process->pid;
    } else if (eax == SYSINT_EXECVE) {
        return threading::currentThread()->process->execve(reinterpret_cast<const char*>(ebx), reinterpret_cast<char *const *>(ecx), reinterpret_cast<char *const *>(edx));
    } else if (eax == SYSINT_WAIT) {
        return threading::currentThread()->process->wait(reinterpret_cast<i32*>(ebx));
    } else if (eax == SYSINT_ISATTY) {
        return threading::currentThread()->process->isatty(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_LSEEK) {
        return threading::currentThread()->process->lseek(reinterpret_cast<i32&>(ebx), reinterpret_cast<i32&>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_FSTAT) {
        return threading::currentThread()->process->fstat(reinterpret_cast<i32&>(ebx),reinterpret_cast<struct stat*>(ebx));
    } else if (eax == SYSINT_SBRK) {
        void* result = threading::currentThread()->process->sbrk(reinterpret_cast<i32&>(ebx));
        return reinterpret_cast<i32&>(result);
    }

    return -1;
}

extern "C" void module_load(Binding* root)
{
    idt.getEntry(0x80).setOffset((u32) sysint_handler);
}