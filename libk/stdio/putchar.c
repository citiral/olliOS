#include "stdio.h"
#include "devices/vga.h"

int putchar(int character)
{
    unsigned char c = (unsigned char)character;
    stdout->write(&c, 1);
    return character;
}
