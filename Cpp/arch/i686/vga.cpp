#include "vga.h"
#include "io.h"

static i16* VGA_POINTER = (i16*)0x000B8000;

static VgaState currentVgaState = {
	column: 0,
	row: 0,
	foregroundColor: VgaColor::LightGrey,
	backgroundColor: VgaColor::Black,
	blinking: false,
};

i16 VgaGenerateEntry(char c) {
	i16 ce = c;
	i16 fe = (i16)currentVgaState.foregroundColor << 8;
	i16 be = (i16)currentVgaState.backgroundColor << 12;
	i16 ble = currentVgaState.blinking << 15;
	return ce |fe | be | ble;
}

void VgaUpdateCursor() {
	i16 position = currentVgaState.column + currentVgaState.row * 80;
	//set position low byte
	outb(0x3D4, 0x0F);
	outb(0x3D5, (u8)position);

	//and position high byte
	outb(0x3D4, 0x0E);
	outb(0x3D5, (u8)(position >> 8));
}

void VgaAdvanceCharacter() {
	currentVgaState.column++;
	if (currentVgaState.column >= VGA_WIDTH) {
		currentVgaState.column = 0;
		currentVgaState.row++;
		if (currentVgaState.row >= VGA_HEIGHT) {
			currentVgaState.row--;
			VgaScrollDown();
			return;
		}
	}
	VgaUpdateCursor();
}

//writes a character to the vga screen. It will correctly handle special characters, such as newlines
void VgaWriteChar(char c) {
	if (c == '\n') {
		currentVgaState.row++;
		currentVgaState.column = 0;
		if (currentVgaState.row >= VGA_HEIGHT) {
			currentVgaState.row--;
			VgaScrollDown();
		} else {
			VgaUpdateCursor();
		}
	} else {
		VgaSetChar(c);
		VgaAdvanceCharacter();
	}
}

//sets the character pointed to by the current vga state to c. This will not advance the vga state, nor will it handle special characters
void VgaSetChar(char c) {
	VGA_POINTER[currentVgaState.column + currentVgaState.row * VGA_WIDTH] = VgaGenerateEntry(c);
}

//sets the character pointed to by the x, y coordinates to c. This will not advance the vga state, nor will it handle special characters
void VgaSetChar(char c, u16 x, u16 y) {
	VGA_POINTER[x + y * VGA_WIDTH] = VgaGenerateEntry(c);
}

//writes the char* as a string until it reaches a null terminated. This will handle special characters
void VgaWriteChars(char* c) {
	while (*c != '\0') {
		VgaWriteChar(*c);
		c++;
	}
}

//clears the vga screen and sets the position to (0, 0)
void VgaClear() {
	VgaSetCursor(0, 0);
	for (i32 i = 0 ; i < VGA_WIDTH * VGA_HEIGHT ; i++)
		VgaWriteChar(' ');
	VgaSetCursor(0, 0);
	VgaUpdateCursor();
}

//sets the vga cursor location to (x, y). It will repeat around the edges (module)
void VgaSetCursor(u16 x, u16 y) {
	currentVgaState.row = y % VGA_HEIGHT;
	currentVgaState.column = x % VGA_WIDTH;
}

//scrolls the vga screen up, moving all text downwards one line. The row pointer is not moved
void VgaScrollUp() {
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		for (i32 y = 1 ; y < VGA_HEIGHT-1 ; y++)
			VgaSetChar(VgaGetChar(x, y-1), x, y);

	VgaUpdateCursor();
}

//scrolls the vga screen down, moving all text upwards one line. The row pointer is not moved
void VgaScrollDown() {
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		for (i32 y = 0 ; y < VGA_HEIGHT-2 ; y++)
			VgaSetChar(VgaGetChar(x, y+1), x, y);

	VgaUpdateCursor();
}

//gets the character at the current cursor location
char VgaGetChar() {
	char character = (char)VGA_POINTER[currentVgaState.column + currentVgaState.row * VGA_WIDTH];
	return character;
}

//gets the character at the given location
char VgaGetChar(u16 x, u16 y) {
	char character = (char)VGA_POINTER[x + y * VGA_WIDTH];
	return character;
}
