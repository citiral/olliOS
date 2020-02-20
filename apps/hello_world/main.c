//#include "libc/libc.h"
#include "types.h"
#include <string.h>

#define SYSINT_OPEN 1
#define SYSINT_CLOSE 2
#define SYSINT_WRITE 3
#define SYSINT_READ 4
#define SYSINT_EXIT 5
#define SYSINT_FORK 6

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

i32 read(i32 file, char *ptr, i32 len) {
  return sysint(SYSINT_READ, *((u32*)&file), (u32)ptr, *((u32*)&len), 0, 0, 0);
}

void exit(i32 status) {
    sysint(SYSINT_EXIT, *((u32*)&status), 0, 0, 0, 0, 0);
}

i32 fork() {
    return sysint(SYSINT_FORK, 0, 0, 0, 0, 0, 0);
}


u8 data[100] = {1, 2, 3};

size_t strlen(const char* str) {
    u32 l = 0;
    while (*(str++)) l++;
    return l;
}

void printf(char* str) {
    i32 file = open("sys/vga", 0, 0);
    write(file, str, strlen(str));
    close(file);
}

int main(int argc, char** argv)
{

    fork();
    i32 pid = fork();
    char t[3];
    t[1] = '\n';
    t[2] = 0;
    printf("test");
    t[0] = '0' + pid;
    printf(t);

    return pid;
    fork();
    fork();
    for (int i = 0 ; fork() && i < 10 ; i++);
    return fork();
    
    return 0;
    
    return 0;

    char buffer[20];

    i32 file = open("dev/ata0/root/usr/include/types.h", 0, 0);
    i32 vga = open("sys/vga", 0, 0);
    
    i32 stat = 1;
    int i = 0;
    while (stat > 0) {
        i++;
        stat = read(file, buffer, 20);
        if (stat > 0) {
            write(vga, buffer, stat);
        }
    }
    close(file);
    close(vga);

    
    /*write(vga, "Hello world!\n", 13);

    for (int i = 0 ; i < 100 ; i++) {
        char c = '0' + (data[i]);
        write(vga, &c, 1);
    }
    data[4] = 4;
    
    close(vga);*/

    return i;
}
