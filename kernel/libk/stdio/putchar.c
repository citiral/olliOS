#include "stdio.h"
#include "bindings.h"

int putchar(int character)
{
    if (stdout == NULL) {
        if (bindings::root != NULL) {
            stdout = bindings::root->get("vga");
        }
    }

    if (stdout != NULL) {
        unsigned char c = (unsigned char)character;
        stdout->write(&c, 1);
        return character;
    }

    return character;
}
