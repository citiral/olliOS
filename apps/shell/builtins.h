#pragma once

#include <stdbool.h>

typedef struct builtin_t {
    const char* name;
    int (*cb)(int pipein, int pipeout, int argc, char** arguments);
} builtin_t;


builtin_t* find_builtin(const char* name);