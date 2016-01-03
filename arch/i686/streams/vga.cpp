#include "vga.h"
#include "io.h"

static i16* VGA_POINTER = (i16*)0x000B8000;

VgaDriver vgaDriver;

VgaDriver::VgaDriver():
	_column(0),
	_row(0),
	_foregroundColor(VgaColor::LightGrey),
	_backgroundColor(VgaColor::Black),
	_blinking(false),
	_vgapointer((i16*)0x000B8000)
{
	//to please the user, the driver is started with a clear screen
	for (int i = 0 ; i < 500 ; i++)
		vgaDriver.write("    ");
	_column = 0;
	_row = 0;
}

VgaDriver::~VgaDriver()
{

}

DeviceType VgaDriver::getDeviceType() const
{
	return DeviceType::Screen;
}

const char* VgaDriver::getDeviceName() const
{
	return VGA_DRIVER_DEVICE_NAME;
}

size_t VgaDriver::write(const void* data, size_t amount)
{
	const char* string = (const char*) data;
	for (size_t i = 0 ; i < amount ; i++) {
		writeChar(string[i]);
	}
	return amount;
}

size_t VgaDriver::write(const void* data)
{
	const char* string = (const char*) data;
	size_t i = 0;
	while (string[i] != '\0')
	{
		writeChar(string[i]);
		i++;
	}

	return i;
}

size_t VgaDriver::read(void* data, size_t amount)
{
	//TODO: read maybe? perhaps do nothing
	return 0;
}

i16 VgaDriver::generateEntry(char c) {
	i16 ce = c;
	i16 fe = (i16)_foregroundColor << 8;
	i16 be = (i16)_backgroundColor << 12;
	i16 ble = _blinking << 15;
	return ce |fe | be | ble;
}

void VgaDriver::updateCursor() {
	i16 position = _column + _row * 80;
	//set position low byte
	outb(0x3D4, 0x0F);
	outb(0x3D5, (u8)position);

	//and position high byte
	outb(0x3D4, 0x0E);
	outb(0x3D5, (u8)(position >> 8));
}

void VgaDriver::advanceCharacter() {
	_column++;
	if (_column >= VGA_WIDTH) {
		_column = 0;
		_row++;
		if (_row >= VGA_HEIGHT) {
			_row--;
			scrollDown();
			return;
		}
	}
	updateCursor();
}

//writes a character to the vga screen. It will correctly handle special characters, such as newlines
void VgaDriver::writeChar(char c) {
	if (c == '\n') {
		_row++;
		_column = 0;
		if (_row >= VGA_HEIGHT) {
			_row--;
			scrollDown();
		} else {
			updateCursor();
		}
	} else {
		setChar(c);
		advanceCharacter();
	}
}

//sets the character pointed to by the current vga state to c. This will not advance the vga state, nor will it handle special characters
void VgaDriver::setChar(char c) {
	_vgapointer[_column + _row * VGA_WIDTH] = generateEntry(c);
}

//sets the character pointed to by the x, y coordinates to c. This will not advance the vga state, nor will it handle special characters
void VgaDriver::setChar(char c, u16 x, u16 y) {
	_vgapointer[x + y * VGA_WIDTH] = generateEntry(c);
}

//writes the char* as a string until it reaches a null terminated. This will handle special characters
void VgaDriver::writeChars(char* c) {
	while (*c != '\0') {
		writeChar(*c);
		c++;
	}
}

//clears the vga screen and sets the position to (0, 0)
void VgaDriver::clear() {
	setCursor(0, 0);
	for (i32 i = 0 ; i < VGA_WIDTH * VGA_HEIGHT ; i++)
		writeChar(' ');
	setCursor(0, 0);
	updateCursor();
}

//sets the vga cursor location to (x, y). It will repeat around the edges (module)
void VgaDriver::setCursor(u16 x, u16 y) {
	_row = y % VGA_HEIGHT;
	_column = x % VGA_WIDTH;
}

//scrolls the vga screen up, moving all text downwards one line. The row pointer is not moved
void VgaDriver::scrollUp() {
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		for (i32 y = 1 ; y < VGA_HEIGHT-1 ; y++)
			setChar(getChar(x, y-1), x, y);

	//clear the top row
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		setChar(' ', x, 0);

	updateCursor();
}

//scrolls the vga screen down, moving all text upwards one line. The row pointer is not moved
void VgaDriver::scrollDown() {
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		for (i32 y = 0 ; y < VGA_HEIGHT-1 ; y++)
			setChar(getChar(x, y+1), x, y);

	//clear the bottom row
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		setChar(' ', x, VGA_HEIGHT-1);

	updateCursor();
}

//gets the character at the current cursor location
char VgaDriver::getChar() {
	char character = (char)_vgapointer[_column + _row * VGA_WIDTH];
	return character;
}

//gets the character at the given location
char VgaDriver::getChar(u16 x, u16 y) {
	char character = (char)_vgapointer[x + y * VGA_WIDTH];
	return character;
}
