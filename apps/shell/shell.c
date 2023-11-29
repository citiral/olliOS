#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include "tokenizer.h"
#include "parser.h"
#include "builtins.h"


const char* find_in_path(const char* target)
{
    if (target[0] == '.' || target[0] == '/') {
        if (access(target, X_OK) == 0) {
            return target;
        } else {
            return NULL;
        }
    } else {
        const char* path = getenv("PATH");
        if (path == NULL) {
            return NULL;
        }
        int pathlength = strlen(path);
        int fulllength = pathlength + strlen(target);

        if (path[pathlength - 1] != '/') {
            fulllength += 1;
        }

        char* full = malloc(sizeof(char) * (fulllength + 1));
        
        strcpy(full, path);
        if (path[pathlength - 1] != '/') {
            full[pathlength] = '/';
            strcpy(full + pathlength + 1, target);
        } else {
            strcpy(full + pathlength, target);
        }

        full[fulllength] = '\0';

        if (access(full, X_OK) == 0) {
            return full;
        } else {
            free(full);
            return NULL;
        }
    }
}


int run_command(int pipein, char** arguments, bool blocking, bool pipeOutput)
{
    int fd[2];
    
    if (pipeOutput) {
        pipe(fd);
    }

    builtin_t* builtin = find_builtin(arguments[0]);
    if (builtin != NULL) {
        int result;
        if (pipeOutput) {
            builtin->cb(pipein, fd[1], get_argument_count(), arguments);
            result = fd[0];
            close(fd[1]);
        } else {
            result = builtin->cb(pipein, 1, get_argument_count(), arguments);
        }

        if (pipein > 0) {
            close(pipein);
        }

        return result;
    }

    int pid = fork();
    if (pid == 0) {
        if (pipeOutput) {
            // make stdout = write end of pipe
            dup2(fd[1], 1);
            // close read end of pipe
            close(fd[0]);
        }

        // if there is a pipe supplied, use that as stdin
        if (pipein > 0) {
            dup2(pipein, 0);
        }

        const char* dest = find_in_path(arguments[0]);
        if (dest == NULL) {
            fprintf(stderr, "%s: command not found\n", arguments[0]);
            exit(-1);
        }
        int status = execve(dest, arguments + 1, environ);
        if (status < 0) {
            fprintf(stderr, "%s: executing failed\n", arguments[0]);
            exit(status);
        }
    } else {
        if (pipein > 0) {
            close(pipein);
        }

        if (pipeOutput) {
            // close write end of pipe
            close(fd[1]);

            if (blocking) {
                int status;
                while (wait(&status) != pid);
            }

            // return read end of pipe
            return fd[0];
        } else if (blocking) {
            int status;
            while (wait(&status) != pid);
            return status;
        } else {
            return 0;
        }
    }

    return -1;
}


void print_prompt(void)
{
    char cwd[1024];

    getcwd(cwd, sizeof(cwd));
    printf("%s: ", cwd);
    fflush(stdout);
}


void prepare_env(void)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    setenv("PATH", cwd, 1);
}


int main(int argc, char** argv)
{
    printf("Welcome to the Ollios Shell!\n");

    prepare_env();

    while (1) {
        print_prompt();
        int status = do_next_command();
        if (status != 0)
            printf("%d\n", status);
    }
}