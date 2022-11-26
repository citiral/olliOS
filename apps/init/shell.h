#pragma once

typedef struct Shell {
    int pid;
    int input;
} Shell;

Shell spawn_shell(char* file);