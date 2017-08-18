#include "streams/vga.h"
#include "streams/keyboard.h"
#include "io.h"

#define COMMAND_PORT 0x64
#define IO_PORT 0x60

#define CMD_SET_COMMAND_BYTE 0x60
#define CMD_SET_SCAMECODE_SET 0xF0

VirtualKeyEvent::VirtualKeyEvent()
{
	this->vkey = VirtualKeycode::INVALID;
	this->status = 0;
}

VirtualKeyEvent::VirtualKeyEvent(VirtualKeycode vkey, u8 status)
{
	this->vkey = vkey;
	this->status = status;
}

KeyboardDriver::KeyboardDriver():
	_code1(0), _code2(0), _code3(0), _bufferPos(0), _bufferLength(0)
{
	//set the command byte to make the keyboard know we are working with interrupts.
	outb(COMMAND_PORT, CMD_SET_COMMAND_BYTE);
	outb(IO_PORT, 0b00000001);
}

KeyboardDriver::~KeyboardDriver()
{

}

DeviceType KeyboardDriver::getDeviceType() const
{
	return DeviceType::Keyboard;
}

void KeyboardDriver::getDeviceInfo(void* deviceinfo) const
{
	DeviceKeyboardInfo* info = (DeviceKeyboardInfo*)deviceinfo;
	info->deviceInfo.name = KEYBOARD_DRIVER_DEVICE_NAME;
}

size_t KeyboardDriver::write(const void* data, size_t amount)
{
	//loop over every character
	const u8* chars = (const u8*)data;
	size_t i;
	for (i = 0 ; i < amount ; i++)
	{
		//add the code
		addCode(chars[i]);

		//check if the current sequence exists
		VirtualKeycode make = convertMakeScancodeToKeycode();
		if (make != VirtualKeycode::INVALID) {
			pushBuffer(VirtualKeyEvent(make, _status | 0b00000001));
			clearCodes();
			updateStatus(VirtualKeyEvent(make, _status | 0b00000001));
		} else
		{
			VirtualKeycode breakcode = convertBreakScancodeToKeycode();
			if (breakcode != VirtualKeycode::INVALID) {
				pushBuffer(VirtualKeyEvent(breakcode, _status | 0b00000000));
				clearCodes();
				updateStatus(VirtualKeyEvent(breakcode, _status | 0b00000000));
			}
		}
		i++;
	}
	return i * sizeof(VirtualKeycode);
}

size_t KeyboardDriver::write(const void* data)
{
	//loop over every character
	const u8* chars = (const u8*)data;
	size_t i = 0;
	while (chars[i] != 0)
	{
		//add the code
		addCode(chars[i]);

		//check if the current sequence exists
		VirtualKeycode make = convertMakeScancodeToKeycode();
		if (make != VirtualKeycode::INVALID) {
			pushBuffer(VirtualKeyEvent(make, _status | 0b00000001));
			clearCodes();
				updateStatus(VirtualKeyEvent(make, _status | 0b00000001));
		} else
		{
			VirtualKeycode breakcode = convertBreakScancodeToKeycode();
			if (breakcode != VirtualKeycode::INVALID) {
				pushBuffer(VirtualKeyEvent(breakcode, _status | 0b00000000));
				clearCodes();
				updateStatus(VirtualKeyEvent(breakcode, _status | 0b00000000));
			}
		}
		i++;
	}
	return i * sizeof(VirtualKeycode);
}

size_t KeyboardDriver::write(char data) {
	//add the code
	addCode(data);

	//check if the current sequence exists
	VirtualKeycode make = convertMakeScancodeToKeycode();
	if (make != VirtualKeycode::INVALID) {
		pushBuffer(VirtualKeyEvent(make, _status | 0b00000001));
		clearCodes();
		updateStatus(VirtualKeyEvent(make, _status | 0b00000001));
	} else
	{
		VirtualKeycode breakcode = convertBreakScancodeToKeycode();
		if (breakcode != VirtualKeycode::INVALID) {
			pushBuffer(VirtualKeyEvent(breakcode, _status | 0b00000000));
			clearCodes();
			updateStatus(VirtualKeyEvent(breakcode, _status | 0b00000000));
		}
	}

	return 1;
}

size_t KeyboardDriver::read(void* data, size_t amount)
{
	VirtualKeyEvent* string = (VirtualKeyEvent*) data;
	size_t i;
	for (i = 0 ; i < amount / sizeof(VirtualKeyEvent) ; i++)
	{
		if (_bufferLength > 0)
		{
			string[i] = popBuffer();
		} else {
			return i * sizeof(VirtualKeyEvent);
		}
	}
	return i * sizeof(VirtualKeyEvent);
}

size_t KeyboardDriver::seek(i32 offset, int position)
{
	//you can't seek in a keyboard, dummy
	return 1;
}

void KeyboardDriver::updateStatus(VirtualKeyEvent code) {
	// if shift is down, replace the shift bit with the make bit of the current code
	if (code.vkey == VirtualKeycode::LSHIFT || code.vkey == VirtualKeycode::RSHIFT) {
		_status &= 0b11111101;
		_status |= ((code.status & 0b00000001) << 1);
	}
}

VirtualKeyEvent KeyboardDriver::popBuffer()
{
	if (_bufferLength <= 0)
		return VirtualKeyEvent(VirtualKeycode::INVALID, 0);
	VirtualKeyEvent key = _buffer[_bufferPos];
	_bufferPos++;
	_bufferLength--;
	_bufferPos %= INPUT_BUFFER_SIZE;
	return key;
}

void KeyboardDriver::addCode(u8 code)
{
	if (_code1 == 0)
		_code1 = code;
	else if (_code2 == 0)
		_code2 = code;
	else if (_code3 == 0)
		_code3 = code;
	else {
		clearCodes();
		addCode(code);
	}

}

void KeyboardDriver::clearCodes()
{
	_code1 = 0;
	_code2 = 0;
	_code3 = 0;
}

void KeyboardDriver::pushBuffer(VirtualKeyEvent key)
{
	if (_bufferLength < INPUT_BUFFER_SIZE) {
		size_t pos = (_bufferPos + _bufferLength) % INPUT_BUFFER_SIZE;
		_buffer[pos] = key;
		_bufferLength++;
	}
}

VirtualKeycode KeyboardDriver::convertMakeScancodeToKeycode() {
	VirtualKeycode k1 = scanset2_map1[_code1];
	if (k1 == VirtualKeycode::INVALID && _code2 != 0) {
		return scanset2_map2[_code2];
	} else {
		return k1;
	}
}

VirtualKeycode KeyboardDriver::convertBreakScancodeToKeycode() {
	//if code1 is F0, or code1 is E0 and code2 is F0, it's a break code
	if (_code1 == 0xF0)
	{
		return scanset2_map1[_code2];
	} else if (_code1 == 0xE0 && _code2 == 0xF0)
	{
		return scanset2_map2[_code3];
	}

	return VirtualKeycode::INVALID;
}

VirtualKeycode scanset2_map1[255] = {
	VirtualKeycode::INVALID,
	VirtualKeycode::F9,		// 01
	VirtualKeycode::F5,
	VirtualKeycode::INVALID,
	VirtualKeycode::F3,
	VirtualKeycode::F1,
	VirtualKeycode::F2,		// 06
	VirtualKeycode::F12,	// 07
	VirtualKeycode::INVALID,
	VirtualKeycode::F10,	// 09
	VirtualKeycode::F8,		// 0A
	VirtualKeycode::F6,
	VirtualKeycode::F4, 	// 0C
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 0F
	VirtualKeycode::INVALID,// 10
	VirtualKeycode::INVALID,
	VirtualKeycode::LSHIFT,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::Q,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 19
	VirtualKeycode::Z,
	VirtualKeycode::S,
	VirtualKeycode::A,		// 1C
	VirtualKeycode::W,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 1F
	VirtualKeycode::INVALID,// 20
	VirtualKeycode::C,
	VirtualKeycode::X,
	VirtualKeycode::D,
	VirtualKeycode::E,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::SPACE,
	VirtualKeycode::V,
	VirtualKeycode::F,
	VirtualKeycode::T,
	VirtualKeycode::R,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 2F
	VirtualKeycode::INVALID,// 30
	VirtualKeycode::N,
	VirtualKeycode::B,
	VirtualKeycode::H,
	VirtualKeycode::G,
	VirtualKeycode::Y,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::M,
	VirtualKeycode::J,
	VirtualKeycode::U,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 40
	VirtualKeycode::INVALID,
	VirtualKeycode::K,
	VirtualKeycode::I,
	VirtualKeycode::O,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::L,
	VirtualKeycode::INVALID,
	VirtualKeycode::P,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 50
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::RSHIFT,
	VirtualKeycode::ENTER,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 60
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::BACKSPACE,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 70
	VirtualKeycode::N_DOT,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::N_MINUS,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
};

VirtualKeycode scanset2_map2[255] = {
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,		// 01
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,		// 06
	VirtualKeycode::INVALID,	// 07
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,	// 09
	VirtualKeycode::INVALID,		// 0A
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID, 	// 0C
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 0F
	VirtualKeycode::INVALID,// 10
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 19
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,		// 1C
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 1F
	VirtualKeycode::INVALID,// 20
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 2F
	VirtualKeycode::INVALID,// 30
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 40
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::N_SLASH,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 50
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
};
