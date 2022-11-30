#include "vga.h"
#include "cpu/io.h"
#include "stdio.h"
#include "cdefs.h"
#include "interfacefile.h"
#include "memory/physical.h"
#include "memory/virtual.h"

static i16* VGA_POINTER = (i16*)0x000B8000;


VgaHandle::VgaHandle(VgaDriver* vga): _vga(vga)
{

}

i32 VgaHandle::write(const void* buffer, size_t size, size_t pos)
{
	return _vga->write(buffer, size);	
}

i32 VgaHandle::read(void* buffer, size_t size, size_t pos) 
{
	return 0;
}

size_t VgaHandle::get_size()
{
	return 0;
}

fs::File* VgaHandle::next_child()
{
	return nullptr;
}
void VgaHandle::reset_child_iterator()
{
}

VgaDriver::VgaDriver(multiboot_info_t* info):
	_column(0),
	_row(0),
	_foregroundColor(VgaColor::LightGrey),
	_backgroundColor(VgaColor::Black),
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

    auto vga = fs::root->get("sys")->create("vga", FILE_CREATE_DIR);

	vga->bind(fs::InterfaceFile::read_only_data<u32>("width", VGA_WIDTH));
	vga->bind(fs::InterfaceFile::read_only_data<u32>("height", VGA_HEIGHT));

	
	vga->bind(fs::InterfaceFile::read_write_data<i8, VgaDriver>("foreground", this, [](VgaDriver* driver) ->  i8 {
		return (i8) driver->_foregroundColor;
	}, [](VgaDriver* driver, i8 color) {
		driver->_foregroundColor = (VgaColor)color;
	}));
	
	vga->bind(fs::InterfaceFile::read_write_data<i8, VgaDriver>("background", this, [](VgaDriver* driver) ->  i8 {
		return (i8) driver->_backgroundColor;
	}, [](VgaDriver* driver, i8 color) {
		driver->_backgroundColor = (VgaColor)color;
	}));
	
	vga->bind(fs::InterfaceFile::read_write_data<i8, VgaDriver>("blinking", this, [](VgaDriver* driver) ->  i8 {
		return (i8) driver->_blinking;
	}, [](VgaDriver* driver, i8 color) {
		driver->_blinking = (bool)color;
	}));
	
	vga->bind(fs::InterfaceFile::read_write_data<u16, VgaDriver>("column", this, [](VgaDriver* driver) ->  u16 {
		return (u16) driver->_column;
	}, [](VgaDriver* driver, u16 column) {
		driver->_column = column;
	}));
	
	vga->bind(fs::InterfaceFile::read_write_data<u16, VgaDriver>("row", this, [](VgaDriver* driver) ->  u16 {
		return (u16) driver->_row;
	}, [](VgaDriver* driver, u16 row) {
		driver->_row = row;
	}));

	auto rawAccess = new fs::InterfaceFile("framebuffer", [](const char* value, size_t length, size_t pos, void* context) {
		VgaDriver* vga = (VgaDriver*) context;
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
		VgaDriver* vga = (VgaDriver*) context;
		if (pos + length > VGA_WIDTH * VGA_HEIGHT) {
			length = (VGA_WIDTH * VGA_HEIGHT) - pos;
		}
		for (size_t i = 0 ; i < length ; i++) {
			buffer[i] = vga->_vgapointer[pos + i] & 0xFF;
		}
		return (i32)length;
	}, this, nullptr);

	vga->bind(rawAccess);
}

VgaDriver::~VgaDriver()
{

}

fs::FileHandle* VgaDriver::open()
{
	return new VgaHandle(this);
}

const char* VgaDriver::get_name()
{
	return "vga";
}

fs::File* VgaDriver::create(const char* name, u32 flags)
{
	return nullptr;
}

fs::File* VgaDriver::bind(fs::File* child)
{
	return nullptr;
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

size_t VgaDriver::write(char data)
{
	UNUSED(data);
	writeChar(data);
	return 1;
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
