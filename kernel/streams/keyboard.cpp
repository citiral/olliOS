#include "keyboard.h"

KeyboardDriver keyboardDriver;

KeyboardDriver::KeyboardDriver():
	_code1(0), _code2(0), _code3(0), _bufferPos(0), _bufferLength(0)
{

}

KeyboardDriver::~KeyboardDriver()
{

}

DeviceType KeyboardDriver::getDeviceType() const
{
	return DeviceType::Keyboard;
}

const char* KeyboardDriver::getDeviceName() const
{
	return VGA_DRIVER_DEVICE_NAME;
}

size_t KeyboardDriver::write(const void* data, size_t amount)
{
	return 0;
}

size_t KeyboardDriver::write(const void* data)
{
	const char* chars = (const char*)data;
	size_t i = 0;
	while (chars[i] != 0)
	{
		i++;
		//TODO
	}
	return 0;
}

size_t KeyboardDriver::read(void* data, size_t amount)
{
	u8* string = (u8*) data;
	size_t i;
	for (i = 0 ; i < amount ; i++)
	{
		if (_bufferLength > 0)
		{
			string[i] = (u8)_buffer[_bufferPos];
			advanceBuffer();
		} else {
			return i;
		}
	}
	return i;
}

void KeyboardDriver::handleKeyInterrupt(unsigned char code)
{
	if (_code1 == 0)
		_code1 = code;
	else if (_code2 == 0)
		_code2 = code;
	else if (_code3 == 0)
		_code3 = code;
}

void KeyboardDriver::advanceBuffer()
{
	_bufferPos++;
	_bufferLength--;
	_bufferPos %= INPUT_BUFFER_SIZE;
}

VirtualKeycode KeyboardDriver::convertMakeScancodeToKeycode() {
	VirtualKeycode code = scanset2_map1[_code1];
	return code;

	//if map1 does not contain the item, check in another map
	//TODO look in map 2 if code1 == e0
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
	VirtualKeycode::INVALID,
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
	VirtualKeycode::INVALID,
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
	VirtualKeycode::INVALID,
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
