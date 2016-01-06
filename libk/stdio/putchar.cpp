#include "stdio.h"
#include "vga.h"

int putchar(int character)
{
    unsigned char c = (unsigned char)character;
    stdout->write(&c, 1);
    return character;
}
