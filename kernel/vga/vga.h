#ifndef __VGA_H
#define __VGA_H

#include "types.h"
#include "file.h"
#include "multiboot.h"

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

class VgaDriver : public fs::File {
public:

	VgaDriver(multiboot_info_t* info);
	virtual ~VgaDriver();

    fs::FileHandle* open();
    const char* get_name();
    fs::File* create(const char* name, u32 flags);
    fs::File* bind(fs::File* child);

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
	VgaColor _foregroundColor;
	VgaColor _backgroundColor;
	bool _blinking;
	i16* _vgapointer;
};

class VgaHandle : public fs::FileHandle {
public:
	VgaHandle(VgaDriver* vga);

	virtual i32 write(const void* buffer, size_t size, size_t pos);
	virtual i32 read(void* buffer, size_t size, size_t pos);
	virtual size_t get_size();

	virtual fs::File* next_child();
	virtual void reset_child_iterator();

private:
	VgaDriver* _vga;
};

#endif /* end of include guard: __VGA_H */
