
#include "file.h"
#include "cpu/interrupt.h"
#include "threading/thread.h"
#include <sys/syscalls.h>
#include <sys/dirent.h>
#include "process.h"
#include "cpu/util.h"


extern "C" void sysint_handler(void);

const char* sysint_names[] = {
    "", "open", "close", "write", "read", "exit", "fork", "getpid", "execve", "wait", "isatty", "lseek", "fstat", "kill", "link", "sbrk", "times", "link"
};

extern "C" i32 sysint_handler_c(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi)
{
    i32 result = -1;
	u16 CS = 0;
	u16 DS = 0;
	u16 SS = 0;
	u32 ESP = 0;

    //printf("SYS: [%s]: %x %x %x %x\n", sysint_names[eax], ebx, ecx, edx, esi);

    if (eax == SYSINT_OPEN) {
        result = threading::currentThread()->process->open(reinterpret_cast<const char*>(ebx), reinterpret_cast<i32&>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_CLOSE) {
        result = threading::currentThread()->process->close(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_WRITE) {
        result = threading::currentThread()->process->write(reinterpret_cast<i32&>(ebx), reinterpret_cast<char *>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_READ) {
        result = threading::currentThread()->process->read(reinterpret_cast<i32&>(ebx), reinterpret_cast<char *>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_EXIT) {
        result = threading::currentThread()->process->exit(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_FORK) {
        result = threading::currentThread()->process->fork();
    } else if (eax == SYSINT_GETPID) {
        result = threading::currentThread()->process->pid;
    } else if (eax == SYSINT_EXECVE) {
        result = threading::currentThread()->process->execve(reinterpret_cast<const char*>(ebx), reinterpret_cast<char *const *>(ecx), reinterpret_cast<char *const *>(edx));
    } else if (eax == SYSINT_WAIT) {
        result = threading::currentThread()->process->wait(reinterpret_cast<i32*>(ebx));
    } else if (eax == SYSINT_ISATTY) {
        result = threading::currentThread()->process->isatty(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_LSEEK) {
        result = threading::currentThread()->process->lseek(reinterpret_cast<i32&>(ebx), reinterpret_cast<i32&>(ecx), reinterpret_cast<i32&>(edx));
    } else if (eax == SYSINT_FSTAT) {
        result = threading::currentThread()->process->fstat(reinterpret_cast<i32&>(ebx),reinterpret_cast<struct stat*>(ebx));
    } else if (eax == SYSINT_SBRK) {
        void* val = threading::currentThread()->process->sbrk(reinterpret_cast<i32&>(ebx));
        result = reinterpret_cast<i32>(val);
    } else if (eax == SYSINT_PIPE) {
        result = threading::currentThread()->process->pipe(reinterpret_cast<int*>(ebx));
    } else if (eax == SYSINT_DUP) {
        result = threading::currentThread()->process->dup(reinterpret_cast<i32&>(ebx));
    } else if (eax == SYSINT_DUP2) {
        result = threading::currentThread()->process->dup2(reinterpret_cast<i32&>(ebx), reinterpret_cast<i32&>(ecx));
    } else if (eax == SYSINT_READDIR) {
        result = threading::currentThread()->process->readdir(reinterpret_cast<i32&>(ebx), reinterpret_cast<struct dirent*>(ecx));
    } else if (eax == SYSINT_GETWD) {
        char* val = threading::currentThread()->process->getwd(reinterpret_cast<char*>(ebx), reinterpret_cast<size_t&>(ecx));
        result = reinterpret_cast<i32>(val);
    }

    //printf("SYS: [%s]: %d\n", sysint_names[eax], result);

    return result;
}

void configure_sysenter_and_syscall()
{
    // Configure sysenter's kernel CS segment to be at offset 8
    // It will assume the kernel DS segment to be at offset CS+8
    // the user CS segment to be at CS+16
    // the user DS segment to be at DS+24
    write_model_specific_register(IA32_SYSENTER_CS, 0, 8);

    // Configure sysenter's entry point to be our sysint_handler
    write_model_specific_register(IA32_SYSENTER_EIP, 0, (u32)sysint_handler);
}

extern "C" void module_load(fs::File* root, const char* argv)
{
    UNUSED(root);
    UNUSED(argv);

    configure_sysenter_and_syscall();

    idt.getEntry(0x80).setOffset((u32) sysint_handler);
}