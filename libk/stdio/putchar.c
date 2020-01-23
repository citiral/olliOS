#include "stdio.h"
#include "fs/bindings.h"
#include "vga.h"

extern VgaDriver* vgaDriver;

int putchar(int character)
{
    if (vgaDriver != NULL) {
        unsigned char c = (unsigned char)character;
        vgaDriver->write(&c, 1);
    }
    return 1;

    /*if (stdout == NULL) {
        if (bindings::root != NULL) {
            stdout = bindings::root->get("vga");
        }
    }

    if (stdout != NULL) {
        unsigned char c = (unsigned char)character;
        stdout->write(1, &c);
        return character;
    }*/
}
