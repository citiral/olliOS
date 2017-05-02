#ifndef __VGA_H
#define __VGA_H

#include "streams/device.h"
#include "types.h"

#define VGA_DRIVER_DEVICE_NAME "VGA"
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

class VgaDriver : public Device {
public:
	VgaDriver();
	virtual ~VgaDriver();

	//gets the type of the vga driver
	virtual DeviceType getDeviceType() const;
	//gets the name of the vga driver
	virtual const char* getDeviceName() const;
	//prints amount bytes to the screen
	virtual size_t write(const void* data, size_t amount);
	//prints bytes to the screen
	virtual size_t write(const void* data);
    //prints a byte to the screen
    virtual size_t write(char data);
	//reads amount bytes from the screen
	virtual size_t read(void* data, size_t amount);
	//changes the position on the screen, counted in characters, starting from the top left,
	//to the bottom right, lines first.
	virtual size_t seek(i32 offset, int position);

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
	//sets the character pointed to by the x, y coordinates to c. This will not advance the vga state, nor will it handle special characters
	void setChar(char c, u16 x, u16 y);
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

extern VgaDriver vgaDriver; //TODO: remove this and map it to the filesystem

#endif /* end of include guard: __VGA_H */
