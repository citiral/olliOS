#include "keyboard.h"

Keyboard::Keyboard() {
	resetKeystates();
	_keyEventCallback = nullptr;
	code1 = 0;
	code2 = 0;
	code3 = 0;
}

void Keyboard::resetKeystates() {
	for (u8 i = 0 ; i < (u8)Keycode::COUNT ; i++) {
		_keystates[i] = false;
	}
}

bool Keyboard::getKeyState(Keycode key) {
	return _keystates[(u8)key];
}

void Keyboard::setKeyState(Keycode key, bool value) {
	_keystates[(u8)key] = value;
	
	//if we have a keyeventcallback, send a keyevent packet 
	if (_keyEventCallback != nullptr) {
		//if we need to send a keypress packet
		if (value) {
			KeypressPacket packet;
			packet.key = key;
			packet.flags = 0;
			_keyEventCallback(packet);
		}
	}
}

void Keyboard::setKeyEventCallback(KeyEventCallback callback) {
	_keyEventCallback = callback;
}

Keycode convertMakeScancodeToKeycode(int code1, int code2, int code3) {
	if (code1 == 0x1C) return Keycode::A;
	else if (code1 == 0x32) return Keycode::B;
	else if (code1 == 0x21) return Keycode::C;
	else if (code1 == 0x23) return Keycode::D;
	return Keycode::INVALID;
}

Keycode convertBreakScancodeToKeycode(int code1, int code2, int code3) {
	return Keycode::INVALID;
}