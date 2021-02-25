#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMD_BUF_LENGTH 512
#define CMD_BUF_PARTS 64

char command_buffer[512];
char* argument_buffer[CMD_BUF_PARTS];

void commit_parameter(int index)
{
    argument_buffer[index] = malloc(strlen(command_buffer) + 1);
    strcpy(argument_buffer[index], command_buffer);
}

void read_command()
{
    char c;
    int argument_index = 0;
    int string_index = 0;

    while ((c = getchar()) != '\n') {
        if (c == ' ') {
            if (string_index > 0) {
                command_buffer[string_index] = '\0';
                commit_parameter(argument_index);
                string_index = 0;
                argument_index++;
            }
        } else {
            command_buffer[string_index] = c;
            if (string_index < CMD_BUF_LENGTH - 1) {
                string_index++;
            }
        }
    }

    if (string_index > 0) {
        command_buffer[string_index] = '\0';
        commit_parameter(argument_index);
    }
}

void free_command_buffer()
{
    for (int i = 0 ; i < CMD_BUF_PARTS ; i++) {
        if (argument_buffer[i] != NULL) {
            free(argument_buffer[i]);
            argument_buffer[i] = NULL;
        }
    }
}

int main(int argc, char** argv)
{
    printf("Welcome to the Ollios Shell!\n");

    memset(command_buffer, 0, sizeof(command_buffer));
    memset(argument_buffer, 0, sizeof(argument_buffer));

    int pipes[2];
    pipe(pipes);

    write(pipes[1], "Test!\n", strlen("Test!\n"));
    close(pipes[1]);
    
    char* a = NULL;
    if (fork() == 0) {
        dup2(pipes[0], 0);
        close(pipes[0]);
        execve("/root/usr/bin/tee", &a, &a);
    } else {
        int stat;
        wait(&stat);
    }

    while (1) {
        printf("> ");
        fflush(stdout);

        read_command();

        if (argument_buffer[0] != NULL) {

            if (strcmp(argument_buffer[0], "exit") == 0) {
                return 0;
            }

            int pid = fork();
            if (pid == 0) {
                return execve(argument_buffer[0], argument_buffer + 1, NULL);
            } else {
                int status;
                wait(&status);
                printf("Process returned with status: %d\n", status);
            }
        }
        free_command_buffer();
    }
}