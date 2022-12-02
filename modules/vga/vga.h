#ifndef _OLLIOS_VGA_H
#define _OLLIOS_VGA_H

#include <stdint.h>

enum class VgaMode {
    Txt,
    Gfx
};

struct Resolution {
    int width;
    int height;
};

enum class FourBitColor: uint8_t {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGrey = 7,
    DarkGrey = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    LightMagenta = 13,
    LightBrown = 14,
    White = 15,
};

#endif
