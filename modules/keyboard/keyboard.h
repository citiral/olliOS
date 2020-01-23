#ifndef __STREAMS_KEYBOARD_H
#define __STREAMS_KEYBOARD_H

#include "types.h"
#include "event.h"
#include "threading/mutex.h"

#define INPUT_BUFFER_SIZE 16

#define SCANSET_GET 0
#define SCANSET_1 1
#define SCANSET_2 2
#define SCANSET_3 3

namespace keyboard {

	void initialize();

	extern VirtualKeycode scanset2_map1[255];
	extern VirtualKeycode scanset2_map2[255];

	class KeyboardDriver {
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

		//makes the driver process amount scancodes from data
		virtual size_t write(const void* data, size_t amount);
		//makes the driver process scancodes from data until a nullpointer is occured
		virtual size_t write(const void* data);
		//makes the driver process a single scancode
		virtual size_t write(char data);
		//reads virtual keycodes from the keyboard buffer
		virtual size_t read(void* data, size_t amount);
		
		threading::Mutex dataMutex;
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

	extern KeyboardDriver* driver;
}

#endif
