#pragma once

#include "types.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum class VgaColor: i8 {
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

struct VgaState {
	u16 column;
	u16 row;
	VgaColor foregroundColor;
	VgaColor backgroundColor;
	bool blinking;
};

//scrolls the vga screen up, moving all text downwards one line. The row pointer is not moved
void VgaScrollUp();
//scrolls the vga screen down, moving all text upwards one line. The row pointer is not moved
void VgaScrollDown();
//writes a character to the vga screen. It will correctly handle special characters, such as newlines
void VgaWriteChar(char c);
//sets the character pointed to by the current vga state to c. This will not advance the vga state, nor will it handle special characters
void VgaSetChar(char c);
//sets the character pointed to by the x, y coordinates to c. This will not advance the vga state, nor will it handle special characters
void VgaSetChar(char c, u16 x, u16 y);
//gets the character at the current cursor location
char VgaGetChar();
//gets the character at the given location
char VgaGetChar(u16 x, u16 y);
//advances the vga pointer by one character.
void VgaAdvanceCharacter();
//writes the char* as a string until it reaches a null terminated. This will handle special characters
void VgaWriteChars(char* c);
//clears the vga screen and sets the position to (0, 0)
void VgaClear();
//sets the vga cursor location to (x, y). It will repeat around the edges (module)
void VgaSetCursor(u16 x, u16 y);
