#pragma once

#include "types.h"
#include "device.h"

#define INPUT_BUFFER_SIZE 16
#define VGA_DRIVER_DEVICE_NAME "KEYBOARD"

enum class VirtualKeycode : u8 {
	INVALID = 0,
	A = 1,
	B = 2,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	SPACE,
	BACKSPACE,
	ENTER,
	LSHIFT,
	RSHIFT,
	LCTRL,
	RCTRL,
	LALT,
	RALT,
};

extern VirtualKeycode scanset2_map1[255];
extern VirtualKeycode scanset2_map2[255];

class KeyboardDriver {
public:
	KeyboardDriver();
	virtual ~KeyboardDriver();

	//gets the type of the keyboard driver
	virtual DeviceType getDeviceType() const;
	//gets the name of the keyboard driver
	virtual const char* getDeviceName() const;
	//makes the driver process amount scancodes from data
	virtual size_t write(const void* data, size_t amount);
	//makes the driver process scancodes from data untill a nullpointer is occured
	virtual size_t write(const void* data);
	//reads virtual keycodes from the keyboard buffer
	virtual size_t read(void* data, size_t amount);

private:
	void advanceBuffer();
	void handleKeyInterrupt(unsigned char code);
	VirtualKeycode convertMakeScancodeToKeycode();
	//VirtualKeycode convertBreakScancodeToKeycode();

	unsigned char _code1;
	unsigned char _code2;
	unsigned char _code3;
	VirtualKeycode _buffer[INPUT_BUFFER_SIZE];
	unsigned char _bufferPos;
	unsigned char _bufferLength;
};

extern KeyboardDriver keyboardDriver; //TODO: remove this and map it to the filesystem
