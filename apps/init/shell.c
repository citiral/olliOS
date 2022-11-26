#include "shell.h"
#include <unistd.h>


Shell spawn_shell(char* file)
{
    Shell shell;
    int fd[2];
    pipe(fd);

    shell.input = fd[1];
    shell.pid = fork();

    if (shell.pid == 0) {
        close(fd[1]);
        dup2(fd[0], 0);

        char* argv[] = {file, NULL};
        execve(file, argv, NULL);
    } else {
        close(fd[0]);
    }
    
    return shell;
}