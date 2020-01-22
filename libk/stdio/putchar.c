#include "stdio.h"
#include "fs/bindings.h"

int putchar(int character)
{
    if (stdout == NULL) {
        if (bindings::root != NULL) {
            stdout = bindings::root->get("vga");
        }
    }

    if (stdout != NULL) {
        unsigned char c = (unsigned char)character;
        stdout->write(1, &c);
        return character;
    }
}
