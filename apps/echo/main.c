//#include "libc/libc.h"
#include <string.h>
#include <stdio.h>

#define SYSINT_OPEN 1
#define SYSINT_CLOSE 2
#define SYSINT_WRITE 3
#define SYSINT_READ 4
#define SYSINT_EXIT 5
#define SYSINT_FORK 6
#define SYSINT_GETPID 7
#define SYSINT_EXECVE 8
/*
i32 sysint(u32 eax, u32 ebx, u32 ecx, u32 edx, u32 esi, u32 edi, u32 ebp);

i32 open(const char* name, i32 flags, i32 mode)
{
    return sysint(SYSINT_OPEN, (u32)name, *((u32*)&flags), *((u32*)&mode), 0, 0, 0);
}

i32 close(i32 file)
{
    return sysint(SYSINT_CLOSE, *((u32*)&file), 0, 0, 0, 0, 0);
}

i32 write(i32 file, char* data, i32 len)
{
    return sysint(SYSINT_WRITE, *((u32*)&file), (u32)data, *((u32*)&len), 0, 0, 0);
}

i32 execve(char *name, char **argv, char **env) {
  return sysint(SYSINT_EXECVE, (u32)name, (u32)argv, (u32)env, 0, 0, 0);
}

size_t strlen(const char* str) {
    u32 l = 0;
    while (*(str++)) l++;
    return l;
}*/
/*
void printf(char* str) {
    i32 file = open("sys/vga", 0, 0);
    write(file, str, strlen(str));
    close(file);
}*/

int main(int argc, char** argv)
{
    
    for (int i = 1 ; i < argc ; i++) {
        printf(argv[i]);
        printf(" ");
    }

    printf("\n");

    return argc;
}
