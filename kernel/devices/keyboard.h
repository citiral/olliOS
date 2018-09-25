#ifndef __STREAMS_KEYBOARD_H
#define __STREAMS_KEYBOARD_H

#include "types.h"
#include "devices/blockdevice.h"

#define INPUT_BUFFER_SIZE 16
#define KEYBOARD_DRIVER_DEVICE_NAME "KEYBOARD"

namespace keyboard {

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
        OPEN_SQUARE,
        CLOSE_SQUARE,
        CAPS_LOCK,
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
        TAB,
		ENTER,
		LSHIFT,
		RSHIFT,
		LCTRL,
		RCTRL,
		LALT,
		RALT,
		N_SLASH,
		N_MINUS,
		N_DOT,
		U_ARROW,
		L_ARROW,
		D_ARROW,
		R_ARROW,
		HOME,
		END,
        TILDE,
        T_0,
        T_1,
        T_2,
        T_3,
        T_4,
        T_5,
        T_6,
        T_7,
        T_8,
        T_9,
        T_10,
        T_11,
        N_0,
        N_1,
        N_2,
        N_3,
        N_4,
        N_5,
        N_6,
        N_7,
        N_8,
        N_9
	};

	void initialize();

	//a single event in the world of virtual keys.
	//the status register is as follows:
	// [0-5: UNUSED | 6: shift | 7: pressed]
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

	class KeyboardDriver : public BlockDevice {
	public:
		KeyboardDriver();
		virtual ~KeyboardDriver();

		void waitForResponse();

		void sendCommand(u8 command);
		void sendDataCommand(u8 command, u8 data);

		void sendKBCommand(u8 command);
		void sendDataKBCommand(u8 command, u8 data);

		// Enable IRQs for this device
		void enableIRQ();
		
		// Executed when an interrupt is fired
		bool interrupt1(u32 interrupt);

		u8 getScanCodeSet();
		void setScanCodeSet(u8 scanset);
		void setScanCodeTranslation(bool enabled);

		//gets the type of the keyboard driver
		virtual DeviceType getDeviceType() const;
		//gets the name of the keyboard driver
		virtual void getDeviceInfo(void* info) const;
		//makes the driver process amount scancodes from data
		virtual size_t write(const void* data, size_t amount);
		//makes the driver process scancodes from data until a nullpointer is occured
		virtual size_t write(const void* data);
		//makes the driver process a single scancode
		virtual size_t write(char data);
		//reads virtual keycodes from the keyboard buffer
		virtual size_t read(void* data, size_t amount);
		//does nothing!
		virtual size_t seek(i32 offset, int position);
	private:
		// add a code to the current scanecode list.
		void addCode(u8 code);
		// clears the current list of scancodes.
		void clearCodes();
		// pushes a keyevent to the buffer of the keyboard driver
		VirtualKeyEvent popBuffer();
		// pops a keyevent from the buffer of the keyboard driver
		void pushBuffer(VirtualKeyEvent key);
		// returns a virtual keycode from the current codes interpreted as make codes, if possible. otherwise,
		// it returns an invalid key.
		VirtualKeycode convertMakeScancodeToKeycode();
		// returns a virtual keycode from the current codes, interpretd as break codes, if possible. otherwise,
		// it returns an invalid key.
		VirtualKeycode convertBreakScancodeToKeycode();
		// Update the internal status with the given virtualkeycode (eg. shift release, press)
		void updateStatus(VirtualKeyEvent code);

		u8 _code1;
		u8 _code2;
		u8 _code3;
		VirtualKeyEvent _buffer[INPUT_BUFFER_SIZE];
		u8 _bufferPos;
		u8 _bufferLength;
		u8 _status;

		volatile bool _noWrite = false; // Disables passing interrupts as key pressed so that it won't mess with return values from config
		volatile int _commandRetValue; // Contains the return value of the last interrupt
		volatile int _commandSendCount = 0; // The number of resends that have happened
		volatile bool _commandReturned = false; // True if the last interrupt was an ACK message. Needs to be reset manually
		volatile bool _commandFailed = false; // True if the last interrupt was a Resend message. Needs to be reset manually
		volatile bool _commandNeedsData = false; // True if the current command also send data
		volatile u8 _commandBufC; // The command to send if a resend is requested
		volatile u8 _commandBufD; // The data to send if a resend is requested (only if _commandNeedsData == true)
		volatile u8 _commandBufP; // The port to send _commandBufC to (_commandBufD will always go to IO_PORT)
	};

}

#endif
