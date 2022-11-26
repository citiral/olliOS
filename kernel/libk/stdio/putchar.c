#include "stdio.h"
#include "file.h"

int putchar(int character)
{
    if (stdout == NULL) {
        if (fs::root != NULL ) {
            fs::File* vga = fs::root->get("dev/vga");
            if (vga) {
                stdout = vga->open();
            }
        }
    }

    if (stdout != NULL) {
        unsigned char c = (unsigned char)character;
        stdout->write(&c, 1, 0);
        return character;
    }

    return character;
}
