#include "parser.h"
#include "shell.h"
#include "tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char* argument_buffer[CMD_BUF_PARTS] = { 0 };
int argument_buffer_index = 0;


void add_token_to_argument_buffer(void)
{
    int length = strlen(get_token_value());
    char* cpy = (char*) malloc(sizeof(char) * length + 1);

    strcpy(cpy, get_token_value());

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


int do_next_command(void)
{
    token_t next_token;
    int status = 0;
    int pipe = 0;

    while (1) {
        next_token = get_next_token();

        if (next_token == STRING) {
            add_token_to_argument_buffer();
        } else if (next_token == EOL) {
            if (argument_buffer_index > 0) {
                status = run_command(pipe, argument_buffer, true, false);
                pipe = 0;
                free_argument_buffer();
                break;
            } else if (pipe > 0) {
                continue;
            } else {
                break;
            }
        } else if (next_token == PIPE) {
            pipe = run_command(pipe, argument_buffer, false, true);
            free_argument_buffer();
        } else if (next_token == SEMICOLON) {
            status = run_command(pipe, argument_buffer, true, false);
            pipe = 0;
            free_argument_buffer();
        } else if (next_token == SUBPROCESS) {
            status = run_command(pipe, argument_buffer, false, false);
            pipe = 0;
            free_argument_buffer();
        } else if (next_token == AND) {
            if (status == 0) {
                status = run_command(pipe, argument_buffer, true, false);
                pipe = 0;
            }
            free_argument_buffer();
        } else if (next_token == OR) {
            int status2 = run_command(pipe, argument_buffer, true, false);
            pipe = 0;
            free_argument_buffer();

            if (status == 0) {
                status = status2;
            }
        }
    }

    if (pipe != 0) {
        close(pipe);
    }

    return status;
}

int get_argument_count(void)
{
    return argument_buffer_index;
}