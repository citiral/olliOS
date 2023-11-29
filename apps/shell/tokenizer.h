#pragma once

#define CMD_BUF_LENGTH 512
#define CMD_BUF_PARTS 64

typedef enum token_t {
    STRING,
    PIPE,
    SEMICOLON,
    EOL,
    SUBPROCESS,
    AND,
    OR,
} token_t;

void init_tokenizer(void);
token_t get_next_token(void);
char* get_token_value(void);