/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

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

extern int sysint(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx, unsigned int esi, unsigned int edi, unsigned int ebp);

void _exit(int status)
{
    sysint(SYSINT_EXIT, *((unsigned int*)&status), 0, 0, 0, 0, 0);
}

int close(int file)
{
    return sysint(SYSINT_CLOSE, *((unsigned int*)&file), 0, 0, 0, 0, 0);
}

char **environ; /* pointer to array of char * strings that define the current environment variables */

int execve(char *name, char **argv, char **env)
{
  return sysint(SYSINT_EXECVE, (unsigned int)name, (unsigned int)argv, (unsigned int)env, 0, 0, 0);
}

int fork()
{
    return sysint(SYSINT_FORK, 0, 0, 0, 0, 0, 0);
}

int fstat(int file, struct stat *st)
{
    return sysint(SYSINT_FSTAT, *(unsigned int*)&file, (unsigned int)st, 0, 0, 0, 0);
}

int getpid()
{
    return sysint(SYSINT_GETPID, 0, 0, 0, 0, 0, 0);
}

int isatty(int file)
{
    return sysint(SYSINT_ISATTY, *((unsigned int*)&file), 0, 0, 0, 0, 0);
}

int kill(int pid, int sig)
{
    return sysint(SYSINT_KILL, *((unsigned int*)&pid), *((unsigned int*)&sig), 0, 0, 0, 0);
}

int link(char *old, char *new)
{
    return sysint(SYSINT_LINK, (unsigned int)old, (unsigned int)new, 0, 0, 0, 0);
}

int lseek(int file, int ptr, int dir)
{
    return sysint(SYSINT_LSEEK, *((unsigned int*)&file), (unsigned int)ptr, *((unsigned int*)&dir), 0, 0, 0);
}

int open(const char *name, int flags, ...)
{
    return sysint(SYSINT_OPEN, (unsigned int)name, *((unsigned int*)&flags), 0, 0, 0, 0);
}

int read(int file, char *ptr, int len)
{
    return sysint(SYSINT_READ, *((unsigned int*)&file), (unsigned int)ptr, *((unsigned int*)&len), 0, 0, 0);
}

caddr_t sbrk(int incr)
{
    int result = sysint(SYSINT_SBRK, *((unsigned int*)&incr), 0, 0, 0, 0, 0);
    return *(caddr_t*)(&result);
}

int stat(const char *file, struct stat *st)
{
    int f = open(file, 0);
    int status = fstat(f, st);
    close(f);

    return status;
}

clock_t times(struct tms *buf)
{
    return sysint(SYSINT_TIMES, (unsigned int)buf, 0, 0, 0, 0, 0);
}

int unlink(char *name)
{
    return sysint(SYSINT_UNLINK, (unsigned int)name, 0, 0, 0, 0, 0);
}

int wait(int *status)
{
  return sysint(SYSINT_WAIT, (unsigned int)status, 0, 0, 0, 0, 0);
}

int write(int file, char *ptr, int len)
{
    return sysint(SYSINT_WRITE, *((unsigned int*)&file), (unsigned int)ptr, *((unsigned int*)&len), 0, 0, 0);
}


