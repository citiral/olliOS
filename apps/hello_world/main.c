//#include "libc/libc.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define SYSINT_OPEN 1
#define SYSINT_CLOSE 2
#define SYSINT_WRITE 3
#define SYSINT_READ 4
#define SYSINT_EXIT 5
#define SYSINT_FORK 6
#define SYSINT_GETPID 7
#define SYSINT_EXECVE 8
#define SYSINT_WAIT 9
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

i32 read(i32 file, char *ptr, i32 len) {
  return sysint(SYSINT_READ, *((u32*)&file), (u32)ptr, *((u32*)&len), 0, 0, 0);
}

void exit(i32 status) {
    sysint(SYSINT_EXIT, *((u32*)&status), 0, 0, 0, 0, 0);
}

i32 fork(void) {
    return sysint(SYSINT_FORK, 0, 0, 0, 0, 0, 0);
}

i32 getpid(void) {
    return sysint(SYSINT_GETPID, 0, 0, 0, 0, 0, 0);
}

i32 execve(char *name, char **argv, char **env) {
  return sysint(SYSINT_EXECVE, (u32)name, (u32)argv, (u32)env, 0, 0, 0);
}

i32 wait(i32 *status) {
  return sysint(SYSINT_WAIT, (u32)status, 0, 0, 0, 0, 0);
}*/
/*
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
}*/


int main(int argc, char** argv)
{    
    for (volatile int i = 0 ; i < 1000000000 ; i++);
    printf("Hello, world!\n");
    return 0;
    //FILE* test = open("/sys/vga", "r");
    //write(test, "abc", 3);
    //close(test);
    char* arg = NULL;
    execve("/root/usr/bin/cat", &arg, &arg);
    return 0;

    for (int i = 0; i < 10 ; i++) {
        int pid = fork();
        if (pid == 0) {

            execve("cat", &arg, &arg);
        } else {
            wait();
        }
    }

    /*if (fork()) {
        wait();
    }*/

    /*while (1) {
        fork();
    }*/
    return getpid();
/*
    char* path = "echo";
    char* args[] = {"echo", "test", "123", 0};
    char** env = NULL;
 
    
    int pid = fork();
    
    if (pid == 0) {
        execve(path, args, env);
    } else {
        return wait(NULL);
    }

    return 1;

    if (fork() == 0) {
        printf("hello child\n");//return execve(path, args, env);
        return 2;
    } else {
        return wait(NULL);
        //printf("hello world\n");
        //return 0;
    }


    for (int i = 0 ; i < argc ; i++) {
        printf("arg: ");
        printf(argv[i]);
        printf("\n");
    }

    fork();
    //i32 pid = fork();
    char t[3];
    t[1] = '\n';
    t[2] = 0;
    printf("test");
    t[0] = '0' + getpid();
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

    

    return i;*/
}
