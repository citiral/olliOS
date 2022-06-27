#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMD_BUF_LENGTH 512
#define CMD_BUF_PARTS 64

typedef enum token_t {
    STRING,
    PIPE,
    SEMICOLON,
    EOL,
} token;

char token_buffer[CMD_BUF_LENGTH];
char* argument_buffer[CMD_BUF_PARTS];
int argument_buffer_index = 0;

char lookAhead = 0;
bool hasLookedAhead = false;

char look_ahead(void)
{
    if (!hasLookedAhead) {
        lookAhead = getchar();
        hasLookedAhead = true;
    }

    return lookAhead;
}

char next_char(void)
{
    if (hasLookedAhead) {
        hasLookedAhead = false;
        return lookAhead;
    } else {
        return getchar();
    }
}

token get_next_token(void)
{
    memset(token_buffer, 0, sizeof(token_buffer));
    int i = 0;

    while (1) {
        char c = next_char();

        if (c == '|') {
            return PIPE;
        } else if (c == '\n') {
            return EOL;
        } else if (c == ';') {
            return SEMICOLON;
        } else if (c == ' ' || c == '\t') {
            continue;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '\\' || c == '/') {
            while (1) {
                if (c == '\\') {
                    c = next_char();
                    token_buffer[i] = c;
                    i++;
                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '/') {
                    token_buffer[i] = c;
                    i++;
                } else {
                    return STRING;
                }

                c = look_ahead();
                if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '\\' || c == '/')) {
                    return STRING;
                } else {
                    c = next_char();
                }
            }
        } else if (c == '"' || c == '\'') {
            char literal = c;
            while (1) {
                c = next_char();
                if (c == '\\') {
                    token_buffer[i] = next_char();
                    i++;
                } else if (c == literal) {
                    return STRING;
                } else {
                    token_buffer[i] = c;
                    i++;
                }
            }
        }
    }
}

void add_token_to_argument_buffer(void)
{
    int length = strlen(token_buffer);
    char* cpy = malloc(sizeof(char) * length + 1);

    strcpy(cpy, token_buffer);

    argument_buffer[argument_buffer_index] = cpy;
    argument_buffer_index++;
}

void free_argument_buffer(void)
{
    for (int i = 0 ; i < argument_buffer_index ; i++) {
        free(argument_buffer[i]);
    }
    memset(argument_buffer, 0, sizeof(argument_buffer));
    argument_buffer_index = 0;
}

int run_command(int pipein)
{
    int pid = fork();
    if (pid == 0) {
        // if there is a pipe supplied, use that as stdin
        if (pipein > 0) {
            dup2(pipein, 0);
        }
        return execve(argument_buffer[0], argument_buffer + 1, NULL);
    } else {
        if (pipein > 0) {
            close(pipein);
        }
        int status;
        while (wait(&status) != pid);
        return 0;
    }
}

int run_command_with_pipe(int pipein)
{
    int fd[2];
    pipe(fd);

    int pid = fork();
    if (pid == 0) {
        // make stdout = write end of pipe
        dup2(fd[1], 1);
        // close read end of pipe
        close(fd[0]);

        // if there is a pipe supplied, use that as stdin
        if (pipein > 0) {
            dup2(pipein, 0);
        }

        // and start process like this
        return execve(argument_buffer[0], argument_buffer + 1, NULL);
    } else {
        if (pipein > 0) {
            close(pipein);
        }

        // close write end of pipe
        close(fd[1]);

        // return read end of pipe
        return fd[0];
    }
}

int do_next_command(void)
{
    token next_token;
    int status = 0;
    int pipe = 0;

    while (1) {
        next_token = get_next_token();

        if (next_token == STRING) {
            add_token_to_argument_buffer();
        } else if (next_token == EOL) {
            if (argument_buffer_index > 0) {
                status = run_command(pipe);
                pipe = 0;
                free_argument_buffer();
                break;
            } else if (pipe > 0) {
                continue;
            } else {
                break;
            }
        } else if (next_token == PIPE) {
            pipe = run_command_with_pipe(pipe);
            free_argument_buffer();
        } else if (next_token == SEMICOLON) {
            status = run_command(pipe);
            pipe = 0;
            free_argument_buffer();
        }
    }

    if (pipe != 0) {
        close(pipe);
    }

    return status;
}

void print_prompt(void)
{
    char cwd[1024];

    getcwd(cwd, sizeof(cwd));
    printf("%s: ", cwd);
    fflush(stdout);
}

int main(int argc, char** argv)
{
    printf("Welcome to the Ollios Shell!\n");

    memset(token_buffer, 0, sizeof(token_buffer));
    memset(argument_buffer, 0, sizeof(argument_buffer));

    while (1) {
        print_prompt();
        int status = do_next_command();
        printf("%d\n", status);
    }
}