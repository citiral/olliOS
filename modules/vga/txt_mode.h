#ifndef __VGA_TEXT_MODE_H
#define __VGA_TEXT_MODE_H

#include "vga.h"
#include "types.h"
#include "multiboot.h"
#include "filesystem/file.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

class TxtModeDriver {
public:

	TxtModeDriver(fs::File* root, multiboot_info_t* info);
	virtual ~TxtModeDriver();

	//prints amount bytes to the screen
	virtual size_t write(const void* data, size_t amount);
	//prints bytes to the screen
	virtual size_t write(const void* data);
	//prints a byte to the screen
	virtual size_t write(char data);

	//sets the character pointed to by the x, y coordinates to c. This will not advance the vga state, nor will it handle special characters
	void setChar(char c, u16 x, u16 y);

private:
	//creates an entry for the vga memory from the given character c and the current state of the vga driver
	i16 generateEntry(char c);
	//scrolls the vga screen up, moving all text downwards one line. The row pointer is not moved
	void scrollUp();
	//scrolls the vga screen down, moving all text upwards one line. The row pointer is not moved
	void scrollDown();
	//writes a character to the vga screen. It will correctly handle special characters, such as newlines
	void writeChar(char c);
	//sets the character pointed to by the current vga state to c. This will not advance the vga state, nor will it handle special characters
	void setChar(char c);
	//gets the character at the current cursor location
	char getChar();
	//gets the character at the given location
	char getChar(u16 x, u16 y);
	//advances the vga pointer by one character.
	void advanceCharacter();
	//writes the char* as a string until it reaches a null terminated. This will handle special characters
	void writeChars(char* c);
	//clears the vga screen and sets the position to (0, 0)
	void clear();
	//sets the vga cursor location to (x, y). It will repeat around the edges (module)
	void setCursor(u16 x, u16 y);
	//updates the rendered location of the cursor on the screen.
	void updateCursor();

	u16 _column;
	u16 _row;
	FourBitColor _foregroundColor;
	FourBitColor _backgroundColor;
	bool _blinking;
	i16* _vgapointer;
};

#endif /* end of include guard: __VGA_TEXT_MODE_H */
