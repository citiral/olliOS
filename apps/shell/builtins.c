#include "builtins.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


int builtin_exit(int pipein, int pipeout, int argc, char** argv)
{
    (void) pipein;
    (void) pipeout;
    (void) argc;
    (void) argv;

    exit(0);
    return -1;
}


int builtin_cd(int pipein, int pipeout, int argc, char** argv)
{
    (void) pipein;
    
    int status;
    
    if (argc > 2) {
        dprintf(pipeout, "cd: too many arguments (%d)\n", argc);
        return -1;
    }

    if (argc == 2) {
        status = chdir(argv[1]);
    } else {
        status = chdir("/");
    }

    return status;
}


int builtin_env(int pipein, int pipeout, int argc, char** argv)
{
    (void) pipein;
    
    char** val = environ;

    while (*val != NULL) {
        dprintf(pipeout, "%s\n", *val);
        val++;
    }

    return 0;
}


int builtin_export(int pipein, int pipeout, int argc, char** argv)
{
    (void) pipein;
    
    if (argc != 2) {
        dprintf(pipeout, "export: expected 1 argument, got (%d)\n", argc);
        return -1;
    }

    char* split = index(argv[1], '=');
    if (split == NULL) {
        dprintf(pipeout, "export: malformed input\n");
        return -1;
    }

    *split = '\0';
    
    setenv(argv[1], split+1, 1);
    
    return 0;
}


int builtin_unset(int pipein, int pipeout, int argc, char** argv)
{
    (void) pipein;
    
    if (argc != 2) {
        dprintf(pipeout, "unset: expected 1 argument, got (%d)\n", argc);
        return -1;
    }

    unsetenv(argv[1]);
    
    return 0;
}


builtin_t builtins[] = {
    {
        .name = "exit",
        .cb = builtin_exit
    },
    {
        .name = "cd",
        .cb = builtin_cd
    },
    {
        .name = "env",
        .cb = builtin_env
    },
    {
        .name = "export",
        .cb = builtin_export
    },
    {
        .name = "unset",
        .cb = builtin_unset
    },
    {
        .name = NULL,
        .cb = NULL
    }
};


builtin_t* find_builtin(const char* name)
{
    size_t i = 0;
    while (builtins[i].name != NULL) {
        if (strcmp(builtins[i].name, name) == 0)
            return builtins + i;
        i++;
    }

    return NULL;
}