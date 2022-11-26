#pragma once

#include "keyboard.h"
#include <stddef.h>

typedef struct InputLogEntry {
    char* value;
    struct InputLogEntry* next;
    struct InputLogEntry* prev;
} InputLogEntry;

typedef struct InputBuffer {
    int out;

    size_t cursorPos;
    char* editBuffer;
    size_t editBufferLength;

    InputLogEntry* log;
    InputLogEntry* selected;
} InputBuffer;

InputBuffer create_input_buffer(int out);
void process_event(InputBuffer* buffer, VirtualKeyEvent event);