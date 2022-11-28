#include "inputbuffer.h"
#include "keyboard.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define LOG_CHUNK_SIZE 32


InputLogEntry* alloc_entry(char* value) {
    InputLogEntry* entry = malloc(sizeof(InputLogEntry));
    entry->value = malloc(strlen(value) + 1);
    entry->next = NULL;
    entry->prev = NULL;

    strcpy(entry->value, value);

    return entry;
}


void expand_char_log(InputBuffer* buffer) {
    buffer->editBufferLength = buffer->editBufferLength * 2;

    char* expanded = malloc(buffer->editBufferLength);
    memset(expanded, 0, buffer->editBufferLength);

    strcpy(expanded, buffer->editBuffer);
    free(buffer->editBuffer);
    buffer->editBuffer = expanded;
}


void add_char_to_log(InputBuffer* buffer, char c) {
    size_t line_length = strlen(buffer->editBuffer);
    if (line_length + 2 >= buffer->editBufferLength) {
        expand_char_log(buffer);
    }

    if (buffer->cursorPos >= line_length) {
        buffer->editBuffer[line_length] = c;
        buffer->editBuffer[line_length + 1] = '\0';
        putchar(c);
    } else {
        for (size_t i = line_length ; i > buffer->cursorPos ; i--) {
            buffer->editBuffer[i+1] = buffer->editBuffer[i];
        }
        buffer->editBuffer[buffer->cursorPos] = c;
        for (size_t i = buffer->cursorPos ; i < line_length + 1 ; i++) {
            putchar(buffer->editBuffer[i]);
        }
        for (size_t i = buffer->cursorPos ; i < line_length ; i++) {
            putchar('\b');
        }
    }
    buffer->cursorPos++;
}


void handle_backspace(InputBuffer* buffer) {
    size_t line_length = strlen(buffer->editBuffer);
    if (buffer->cursorPos > 0) {
        putchar('\b');
        for (size_t i = buffer->cursorPos ; i <= line_length ; i++) {
            buffer->editBuffer[i-1] = buffer->editBuffer[i];
            putchar(buffer->editBuffer[i-1]);
        }
        
        for (size_t i = buffer->cursorPos ; i <= line_length ; i++) {
            putchar('\b');
        }

        buffer->cursorPos--;
    }
}


void handle_delete(InputBuffer* buffer) {
    size_t line_length = strlen(buffer->editBuffer);
    if (buffer->cursorPos < line_length) {
        for (size_t i = buffer->cursorPos ; i < line_length ; i++) {
            buffer->editBuffer[i] = buffer->editBuffer[i+1];
            putchar(buffer->editBuffer[i]);
        }
        putchar(' ');
        
        for (size_t i = buffer->cursorPos ; i <= line_length ; i++) {
            putchar('\b');
        }
    }
}


void commit_input(InputBuffer* buffer) {
    size_t written = 0;
    size_t length = strlen(buffer->editBuffer);
    while (written < length) {
        written += write(buffer->out, buffer->editBuffer + written, length - written);
    }
    putchar('\n');
    write(buffer->out, "\n", 1);

    InputLogEntry* entry = alloc_entry(buffer->editBuffer);
    buffer->selected = NULL;
    if (buffer->log == NULL) {
        buffer->log = entry;
    } else {
        buffer->log->prev = entry;
        entry->next = buffer->log;
        buffer->log = entry;
    }

    buffer->editBuffer[0] = 0;
    buffer->cursorPos = 0;
}


InputBuffer create_input_buffer(int out) {
    InputBuffer buf;
    buf.out = out;
    buf.cursorPos = 0;
    buf.editBuffer = malloc(LOG_CHUNK_SIZE);
    memset(buf.editBuffer, 0, LOG_CHUNK_SIZE);
    buf.editBufferLength = LOG_CHUNK_SIZE;
    buf.log = NULL;
    buf.selected = NULL;
    return buf;
}

void switch_to_entry(InputBuffer* buffer, InputLogEntry* entry) {
    buffer->selected = entry;

    size_t length = strlen(buffer->editBuffer);
    for (size_t i = buffer->cursorPos ; i < length ; i++) {
        putchar(buffer->editBuffer[i]);
    }

    for (size_t i = 0 ; i < length ; i++) {
        printf("\b \b");
    }

    if (entry == NULL) {
        buffer->cursorPos = 0;
        buffer->editBuffer[0] = 0;
    } else {
        strcpy(buffer->editBuffer, buffer->selected->value);
        buffer->cursorPos = strlen(buffer->editBuffer);
        printf("%s", buffer->editBuffer);
    }
}


void process_event(InputBuffer* buffer, VirtualKeyEvent event) {
    if (is_pressed(event)) {
        char parsed;
        if (event.vkey == ENTER && !is_shift_modifier(event))
        {
            commit_input(buffer);
        }
        else if (event.vkey == L_ARROW)
        {
            if (buffer->cursorPos > 0) {
                buffer->cursorPos--;
                printf("\b");
            }
        }
        else if (event.vkey == R_ARROW)
        {
            if (buffer->cursorPos < strlen(buffer->editBuffer)) {
                putchar(buffer->editBuffer[buffer->cursorPos]);
                buffer->cursorPos++;
            }
        }
        else if (event.vkey == BACKSPACE)
        {
            handle_backspace(buffer);
        }
        else if (event.vkey == DELETE)
        {
            handle_delete(buffer);
        }
        else if (event.vkey == U_ARROW)
        {
            if (buffer->selected == NULL) {
                switch_to_entry(buffer, buffer->log);
            } else {
                switch_to_entry(buffer, buffer->selected->next);
            }
        }
        else if (event.vkey == D_ARROW)
        {
            if (buffer->selected == NULL) {
                switch_to_entry(buffer, buffer->log);
            } else {
                switch_to_entry(buffer, buffer->selected->next);
            }
        }
        else if (keyevent_to_char(event, &parsed))
        {
            add_char_to_log(buffer, parsed);
        }
    }
}