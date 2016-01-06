#pragma once

#include "types.h"
#include "device.h"

#define INPUT_BUFFER_SIZE 16
#define KEYBOARD_DRIVER_DEVICE_NAME "KEYBOARD"

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

//a single event in the world of virtual keys.
//the status register is as follows:
// [0-6: UNUSED | 7: presed]
// make: 1 if the key is pressed, 0 if the key is released

typedef struct __attribute__ ((__packed__))  VirtualKeyEvent {
	VirtualKeycode vkey;
	u8 status;

public:
	VirtualKeyEvent();
	VirtualKeyEvent(VirtualKeycode vkey, u8 status);
} VirtualKeyEvent;

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
	//does nothing!
    virtual void seek(i32 offset, int position);

private:
	// add a code to the current scanecode list.
	void addCode(u8 code);
	// clears the current list of scancodes.
	void clearCodes();
	// pushes a keyevent to the buffer of the keyboard driver
	VirtualKeyEvent popBuffer();
	// pops a keyevent from the buffer of the keyboard driver
	void pushBuffer(VirtualKeyEvent key);
	// handles a single scancode from the keyboard
	void handleScanCode(unsigned char code);
	// returns a virtual keycode from the current codes interpreted as make codes, if possible. otherwise,
	// it returns an invalid key.
	VirtualKeycode convertMakeScancodeToKeycode();
	// returns a virtual keycode from the current codes, interpretd as break codes, if possible. otherwise,
	// it returns an invalid key.
	VirtualKeycode convertBreakScancodeToKeycode();

	u8 _code1;
	u8 _code2;
	u8 _code3;
	VirtualKeyEvent _buffer[INPUT_BUFFER_SIZE];
	u8 _bufferPos;
	u8 _bufferLength;
};

extern KeyboardDriver keyboardDriver; //TODO: remove this and map it to the filesystem
