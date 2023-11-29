#include "tokenizer.h"
#include <stdbool.h>

char token_buffer[CMD_BUF_LENGTH];
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

token_t get_next_token(void)
{
    memset(token_buffer, 0, sizeof(token_buffer));
    int i = 0;

    while (1) {
        char c = next_char();

        if (c == '|') {
            return look_ahead() == '|' ? OR : PIPE;
        } else if (c == '&') {
            return look_ahead() == '&' ? AND : SUBPROCESS;
        } else if (c == '\n') {
            return EOL;
        } else if (c == ';') {
            return SEMICOLON;
        } else if (c == ' ' || c == '\t') {
            continue;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '\\' || c == '/' || c == '.' || c == '=') {
            while (1) {
                if (c == '\\') {
                    c = next_char();
                    token_buffer[i] = c;
                    i++;
                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '/' || c == '.' || c == '=') {
                    token_buffer[i] = c;
                    i++;
                } else {
                    return STRING;
                }

                c = look_ahead();
                if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '\\' || c == '/' || c == '.' || c == '=')) {
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

char* get_token_value(void)
{
    return token_buffer;
}