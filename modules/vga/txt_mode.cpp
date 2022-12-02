#include "txt_mode.h"
#include "cpu/io.h"
#include "stdio.h"
#include "cdefs.h"
#include "interfacefile.h"
#include "memory/physical.h"
#include "memory/virtual.h"


static i16* VGA_POINTER = (i16*)0x000B8000;

TxtModeDriver::TxtModeDriver(fs::File* root, multiboot_info_t* info):
	_column(0),
	_row(0),
	_foregroundColor(FourBitColor::LightGrey),
	_backgroundColor(FourBitColor::Black),
	_blinking(false),
	_vgapointer(VGA_POINTER)
{
	/*memory::physicalMemoryManager.reservePhysicalMemory((void*)info->framebuffer_addr, info->framebuffer_pitch * info->framebuffer_width);
	
	size_t fbSize = info->framebuffer_pitch * info->framebuffer_height;

	for (int x = 0 ; x < fbSize ; x += 4096) {
		char* virt = (char*)memory::kernelPageDirectory.bindPhysicalPage(((char*)info->framebuffer_addr) + x, memory::UserMode::Supervisor);
		for (unsigned int i = 0 ; i < 4096 ; i++) {
			virt[i] = i;
		}
	}*/

	//to please the user, the driver is started with a clear screen
	for (int i = 0 ; i < 500 ; i++)
		write("    ");
	_column = 0;
	_row = 0;
	
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | 0);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);

    auto vgafile = root->get("sys")->create("vga", FILE_CREATE_DIR);

	vgafile->bind(fs::InterfaceFile::read_only_data<u32>("width", VGA_WIDTH));
	vgafile->bind(fs::InterfaceFile::read_only_data<u32>("height", VGA_HEIGHT));

	
	vgafile->bind(fs::InterfaceFile::read_write_data<i8, TxtModeDriver>("foreground", this, [](TxtModeDriver* driver) ->  i8 {
		return (i8) driver->_foregroundColor;
	}, [](TxtModeDriver* driver, i8 color) {
		driver->_foregroundColor = (FourBitColor)color;
	}));
	
	vgafile->bind(fs::InterfaceFile::read_write_data<i8, TxtModeDriver>("background", this, [](TxtModeDriver* driver) ->  i8 {
		return (i8) driver->_backgroundColor;
	}, [](TxtModeDriver* driver, i8 color) {
		driver->_backgroundColor = (FourBitColor)color;
	}));
	
	vgafile->bind(fs::InterfaceFile::read_write_data<i8, TxtModeDriver>("blinking", this, [](TxtModeDriver* driver) ->  i8 {
		return (i8) driver->_blinking;
	}, [](TxtModeDriver* driver, i8 color) {
		driver->_blinking = (bool)color;
	}));
	
	vgafile->bind(fs::InterfaceFile::read_write_data<u16, TxtModeDriver>("column", this, [](TxtModeDriver* driver) ->  u16 {
		return (u16) driver->_column;
	}, [](TxtModeDriver* driver, u16 column) {
		driver->_column = column;
	}));
	
	vgafile->bind(fs::InterfaceFile::read_write_data<u16, TxtModeDriver>("row", this, [](TxtModeDriver* driver) ->  u16 {
		return (u16) driver->_row;
	}, [](TxtModeDriver* driver, u16 row) {
		driver->_row = row;
	}));

	vgafile->bind(new fs::InterfaceFile("framebuffer", [](const char* value, size_t length, size_t pos, void* context) {
		TxtModeDriver* vga = (TxtModeDriver*) context;
		if (pos + length > VGA_WIDTH * VGA_HEIGHT) {
			length = (VGA_WIDTH * VGA_HEIGHT) - pos;
		}
		for (size_t i = 0 ; i < length ; i++) {
			size_t x = (pos + i) % VGA_WIDTH;
			size_t y = (pos + i) / VGA_WIDTH;
			vga->setChar(value[i], x, y);
		}
		return (i32)length;
	}, [](char* buffer, size_t length, size_t pos, void* context) {
		TxtModeDriver* vga = (TxtModeDriver*) context;
		if (pos + length > VGA_WIDTH * VGA_HEIGHT) {
			length = (VGA_WIDTH * VGA_HEIGHT) - pos;
		}
		for (size_t i = 0 ; i < length ; i++) {
			buffer[i] = vga->_vgapointer[pos + i] & 0xFF;
		}
		return (i32)length;
	}, this, nullptr));

	root->get("dev")->bind(new fs::InterfaceFile("vga", [](const char* value, size_t length, size_t pos, void* context) {
		TxtModeDriver* vga = (TxtModeDriver*) context;
		vga->write(value, length);
		return (i32)length;
	}, [](char* buffer, size_t length, size_t pos, void* context) {
		return 0;
	}, this, nullptr));
}

TxtModeDriver::~TxtModeDriver()
{

}

size_t TxtModeDriver::write(const void* data, size_t amount)
{
	const char* string = (const char*) data;
	for (size_t i = 0 ; i < amount ; i++) {
		writeChar(string[i]);
	}
	return amount;
}

size_t TxtModeDriver::write(const void* data)
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

size_t TxtModeDriver::write(char data)
{
	UNUSED(data);
	writeChar(data);
	return 1;
}

i16 TxtModeDriver::generateEntry(char c) {
	i16 ce = c;
	i16 fe = (i16)_foregroundColor << 8;
	i16 be = (i16)_backgroundColor << 12;
	i16 ble = _blinking << 15;
	return ce |fe | be | ble;
}

void TxtModeDriver::updateCursor() {
	i16 position = _column + _row * 80;
	//set position low byte
	outb(0x3D4, 0x0F);
	outb(0x3D5, (u8)position);

	//and position high byte
	outb(0x3D4, 0x0E);
	outb(0x3D5, (u8)(position >> 8));
}

void TxtModeDriver::advanceCharacter() {
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
void TxtModeDriver::writeChar(char c) {
	if (c == '\n') {
		_row++;
		_column = 0;
		if (_row >= VGA_HEIGHT) {
			_row--;
			scrollDown();
		} else {
			updateCursor();
		}
	} else if (c == '\t') {
		int left = 4 - (_column % 4);
		for (int i = 0 ; i < left ; i++) {
			setChar(' ');
			advanceCharacter();
		}
	} else if (c == '\r') {
		_column = 0;
		updateCursor();
	} else if (c == '\b') {
		if (_column > 0) {
			_column--;
		} else if (_row > 0) {
			_row--;
			_column = VGA_WIDTH - 1;
		}
		updateCursor();
	} else {
		setChar(c);
		advanceCharacter();
	}
}

//sets the character pointed to by the current vga state to c. This will not advance the vga state, nor will it handle special characters
void TxtModeDriver::setChar(char c) {
	_vgapointer[_column + _row * VGA_WIDTH] = generateEntry(c);
}

//sets the character pointed to by the x, y coordinates to c. This will not advance the vga state, nor will it handle special characters
void TxtModeDriver::setChar(char c, u16 x, u16 y) {
	_vgapointer[x + y * VGA_WIDTH] = generateEntry(c);
}

//writes the char* as a string until it reaches a null terminated. This will handle special characters
void TxtModeDriver::writeChars(char* c) {
	while (*c != '\0') {
		writeChar(*c);
		c++;
	}
}

//clears the vga screen and sets the position to (0, 0)
void TxtModeDriver::clear() {
	setCursor(0, 0);
	for (i32 i = 0 ; i < VGA_WIDTH * VGA_HEIGHT ; i++)
		writeChar(' ');
	setCursor(0, 0);
	updateCursor();
}

//sets the vga cursor location to (x, y). It will repeat around the edges (module)
void TxtModeDriver::setCursor(u16 x, u16 y) {
	_row = y % VGA_HEIGHT;
	_column = x % VGA_WIDTH;
}

//scrolls the vga screen up, moving all text downwards one line. The row pointer is not moved
void TxtModeDriver::scrollUp() {
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		for (i32 y = 1 ; y < VGA_HEIGHT-1 ; y++)
			setChar(getChar(x, y-1), x, y);

	//clear the top row
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		setChar(' ', x, 0);

	updateCursor();
}

//scrolls the vga screen down, moving all text upwards one line. The row pointer is not moved
void TxtModeDriver::scrollDown() {
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		for (i32 y = 0 ; y < VGA_HEIGHT-1 ; y++)
			setChar(getChar(x, y+1), x, y);

	//clear the bottom row
	for (i32 x = 0 ; x < VGA_WIDTH ; x++)
		setChar(' ', x, VGA_HEIGHT-1);

	updateCursor();
}

//gets the character at the current cursor location
char TxtModeDriver::getChar() {
	char character = (char)_vgapointer[_column + _row * VGA_WIDTH];
	return character;
}

//gets the character at the given location
char TxtModeDriver::getChar(u16 x, u16 y) {
	char character = (char)_vgapointer[x + y * VGA_WIDTH];
	return character;
}
