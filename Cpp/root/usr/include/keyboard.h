#pragma once
#include "types.h"

#define INPUT_BUFFER_SIZE 255

enum class Keycode : u8 {
	A = 0,
	B = 1,
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
	SPACE,
	BACKSPACE,
	ENTER,
	COUNT,
	INVALID,
};

struct KeypressPacket {
	Keycode key;
	u8 flags; //FLAGS WILL BE (shift, ctrl, alt, 0, 0, 0, 0, 0)
};

typedef void (*KeyEventCallback)(KeypressPacket);

class Keyboard {
public:
	Keyboard();
	void resetKeystates();
	bool getKeyState(Keycode key);
	void setKeyState(Keycode key, bool value);
	void setKeyEventCallback(KeyEventCallback callback);
	void handleKeyInterrupt(int code);
	
private:
	int code1;
	int code2;
	int code3;
	
	bool _keystates[(u8)Keycode::COUNT];
	KeyEventCallback _keyEventCallback;
};

//the keyboard that is used
static Keyboard keyBoard;

Keycode convertMakeScancodeToKeycode(int code1, int code2, int code3);
Keycode convertBreakScancodeToKeycode(int code1, int code2, int code3);