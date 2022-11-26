#include "keyboard.h"
#include "cdefs.h"
#include "stdio.h"
#include "cpu/io.h"
#include "cpu/interrupt.h"
#include "threading/semaphore.h"
#include "threading/scheduler.h"

#define DATA_PORT    0x60
#define STATUS_PORT  0x64
#define COMMAND_PORT 0x64

#define PS2_READ_CONFIG 0x20
#define PS2_WRITE_CONFIG 0x60
#define KB_SCANCODE_SET 0xF0

#define KB_ACK 0xFA

#define BIT_SCANCODE_TRANSLATION (1<<6)

#define STATUS_INPUT_BUFFER_EMPTY (1<<1)
#define STATUS_INPUT_RESPONSE_READY (1<<0)

extern "C" void intHandlerKeyboard_asm(void);

void ps2_write_command(u8 cmd)
{
	// wait on the status register to be clear
	while (inb(STATUS_PORT) & (STATUS_INPUT_BUFFER_EMPTY)) for (volatile int i = 0 ; i < 1000 ; i++);
	outb(COMMAND_PORT, cmd);
}

void ps2_write_command2(u8 cmd, u8 cmd2)
{
	ps2_write_command(cmd);

	// wait on the status register to be clear
	while (inb(STATUS_PORT) & (STATUS_INPUT_BUFFER_EMPTY)) for (volatile int i = 0 ; i < 1000 ; i++);
	outb(DATA_PORT, cmd2);
}

u8 ps2_write_command_with_response(u8 cmd)
{
	ps2_write_command(cmd);

	// wait on the status register to be clear
	while (!(inb(STATUS_PORT) & (STATUS_INPUT_RESPONSE_READY))) for (volatile int i = 0 ; i < 1000 ; i++);
	u8 r = inb(DATA_PORT);
	return r;
}

u8 ps2_write_command2_with_response(u8 cmd, u8 cmd2)
{
	ps2_write_command2(cmd, cmd2);

	// wait on the status register to be clear
	while (!(inb(STATUS_PORT) & (STATUS_INPUT_RESPONSE_READY))) for (volatile int i = 0 ; i < 1000 ; i++);
	return inb(DATA_PORT);
}

void ps2_write_data(u8 cmd)
{
	// wait on the status register to be clear
	while (inb(STATUS_PORT) & (STATUS_INPUT_BUFFER_EMPTY)) for (volatile int i = 0 ; i < 1000 ; i++);
	outb(DATA_PORT, cmd);
}

void ps2_write_data2(u8 cmd, u8 cmd2)
{
	ps2_write_data(cmd);
	ps2_write_data(cmd2);
}

u8 ps2_write_data_with_response(u8 cmd)
{
	ps2_write_data(cmd);

	// wait on the status register to be clear
	while (!(inb(STATUS_PORT) & (STATUS_INPUT_RESPONSE_READY))) for (volatile int i = 0 ; i < 1000 ; i++);
	u8 r = inb(DATA_PORT);
	return r;
}

u8 ps2_write_data2_with_response(u8 cmd, u8 cmd2)
{
	ps2_write_data2(cmd, cmd2);

	// wait on the status register to be clear
	while (!(inb(STATUS_PORT) & (STATUS_INPUT_RESPONSE_READY))) for (volatile int i = 0 ; i < 1000 ; i++);
	return inb(DATA_PORT);
}

u8 ps2_get_response()
{
	// wait on the status register to be clear
	while (!(inb(STATUS_PORT) & (STATUS_INPUT_RESPONSE_READY))) for (volatile int i = 0 ; i < 1000 ; i++);
	return inb(DATA_PORT);
}


namespace keyboard {

KeyboardDriver* driver = 0;


VirtualKeyEvent MakeVirtualKeyEvent()
{
	VirtualKeyEvent key;
	key.vkey = VirtualKeycode::INVALID;
	key.status = 0;
	return key;
}

VirtualKeyEvent MakeVirtualKeyEvent(VirtualKeycode vkey, u8 status)
{
	VirtualKeyEvent key;
	key.vkey = vkey;
	key.status = status;
	return key;
}

KeyboardDriver::KeyboardDriver():
	_code1(0), _code2(0), _code3(0), _bufferPos(0), _bufferLength(0), _status(0), dataMutex()
{
	_ignoreInterrupts = false;
	enableIRQ();
}

KeyboardDriver::~KeyboardDriver()
{

}

void KeyboardDriver::init()
{
	_ignoreInterrupts = true;
	printf("PS/2 config byte: %x\n", ps2_write_command_with_response(0x20));
	ps2_write_command2(PS2_WRITE_CONFIG, 0b00000101);
	printf("PS/2 config byte: %x\n", ps2_write_command_with_response(0x20));
	ps2_write_command(0xAE);
	_ignoreInterrupts = false;

	return;

	// empty keyboard buffer
	unsigned char kbb = 0;
	while(((kbb = inb(COMMAND_PORT)) & 1) == 1)
	{
		inb(DATA_PORT);
	}
}

extern "C" void intHandlerKeyboard(u32 interrupt)
{
    // send it to the keyboarddriver
	driver->interrupt1(interrupt);

    // and end the interrupt
	end_interrupt(interrupt);
}

void KeyboardDriver::enableIRQ()
{
	idt.getEntry(INT_KEYBOARD).setOffset((u32)&intHandlerKeyboard_asm);
}

void KeyboardDriver::waitForResponse()
{
	while (!_commandReturned && !_commandFailed) ;
}

void KeyboardDriver::sendCommand(u8 command)
{
	_commandNeedsData = false;
	_commandBufD = command;
	_commandBufP = command;
	_commandSendCount = 0;
	_commandReturned = false;
	_commandFailed = false;
	//printf("Sending 0x%X\n", command);

	while (inb(STATUS_PORT) & (STATUS_INPUT_BUFFER_EMPTY)) for (volatile int i = 0 ; i < 1000 ; i++);
	outb(COMMAND_PORT, command);
}

void KeyboardDriver::sendDataCommand(u8 command, u8 data)
{
	_commandNeedsData = false;
	_commandBufC = command;
	_commandBufD = data;
	_commandBufP = COMMAND_PORT;
	_commandSendCount = 0;
	_commandReturned = false;
	_commandFailed = false;
	//printf("Sending 0x%X 0x%X\n", command, data);

	while (inb(STATUS_PORT) & (STATUS_INPUT_BUFFER_EMPTY)) for (volatile int i = 0 ; i < 1000 ; i++);
	outb(COMMAND_PORT, command);
	while (inb(STATUS_PORT) & (STATUS_INPUT_BUFFER_EMPTY)) for (volatile int i = 0 ; i < 1000 ; i++);
	outb(DATA_PORT, data);
}

void KeyboardDriver::sendKBCommand(u8 command)
{
	_commandNeedsData = false;
	_commandBufC = command;
	_commandBufP = DATA_PORT;
	_commandSendCount = 0;
	_commandReturned = false;
	_commandFailed = false;
	
	printf("Sending KB 0x%X\n", command);
	outb(DATA_PORT, command);
}

void KeyboardDriver::sendDataKBCommand(u8 command, u8 data)
{
	sendKBCommand(command);
	while (!_commandReturned && !_commandFailed) ;
	if (_commandReturned)
	{
		sendKBCommand(data);
		while (!_commandReturned && !_commandFailed) ;
	}
}

bool KeyboardDriver::interrupt1(u32 interrupt) {
	if (_ignoreInterrupts == true) {
		return true;
	}

	UNUSED(interrupt);
	u8 data = inb(DATA_PORT);
	_commandRetValue = data;
	if (data == 0xFA) // Received acknowledge
		_commandReturned = true;
	else if (data == 0xFE) { // Something happened, resend
		// We'll only retry 3 times. If it still fails then the
		// controller probably just doesn't support the command
		if (_commandSendCount < 3) {
			_commandSendCount++;
			outb(_commandBufP, _commandBufC);
			if (_commandNeedsData)
				// Data always goes through IO port (afaik)
				outb(DATA_PORT, _commandBufD);
		}
		else
			_commandFailed = true;
	}
	else if (!_noWrite)
	{
		write(data);
	}
	else
	{
		_commandReturned = true;
	}
	
	dataMutex.release();
	return true;
}

u8 KeyboardDriver::getScanCodeSet(){
	_ignoreInterrupts = true;
	u8 resp = ps2_write_data_with_response(0xF0);
	if (resp != KB_ACK) {
		printf("Keyboard invalid response: %X\n", resp);
	}
	resp = ps2_write_data_with_response(0x00);
	if (resp != KB_ACK) {
		printf("Keyboard invalid response: %X\n", resp);
	}
	u8 scancode = ps2_get_response();

	_ignoreInterrupts = false;
	return scancode;
}

void KeyboardDriver::setScanCodeSet(u8 scanset) {
	u8 current = getScanCodeSet();	
	printf("current scancodeset: %d\n", current);
	
	_ignoreInterrupts = true;
	u8 resp = ps2_write_data_with_response(0xF0);
	if (resp != KB_ACK) {
		printf("Keyboard invalid response: %X\n", resp);
	}
	resp = ps2_write_data_with_response(scanset);
	if (resp != KB_ACK) {
		printf("Keyboard invalid response: %X\n", resp);
	}
	_ignoreInterrupts = false;

	printf("current scancodeset: %d\n", getScanCodeSet());
}

void KeyboardDriver::setScanCodeTranslation(bool enabled) {
	// Read controller configuration
	_noWrite = true;
	sendCommand(0x20);
	waitForResponse();
	_noWrite = false;
	u8 config = _commandRetValue;

	if (enabled)
		// Clear bit
		config = (config | BIT_SCANCODE_TRANSLATION);
	else
		// Set bit
		config = (config & ~BIT_SCANCODE_TRANSLATION);
	
	_noWrite = true;
	sendDataCommand(0x60, config);
	_noWrite = false;
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
			pushBuffer(MakeVirtualKeyEvent(make, _status | 0b00000001));
			clearCodes();
			updateStatus(MakeVirtualKeyEvent(make, _status | 0b00000001));
		} else
		{
			VirtualKeycode breakcode = convertBreakScancodeToKeycode();
			if (breakcode != VirtualKeycode::INVALID) {
				pushBuffer(MakeVirtualKeyEvent(breakcode, _status | 0b00000000));
				clearCodes();
				updateStatus(MakeVirtualKeyEvent(breakcode, _status | 0b00000000));
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
			pushBuffer(MakeVirtualKeyEvent(make, _status | 0b00000001));
			clearCodes();
			updateStatus(MakeVirtualKeyEvent(make, _status | 0b00000001));
		} else
		{
			VirtualKeycode breakcode = convertBreakScancodeToKeycode();
			if (breakcode != VirtualKeycode::INVALID) {
				pushBuffer(MakeVirtualKeyEvent(breakcode, _status | 0b00000000));
				clearCodes();
				updateStatus(MakeVirtualKeyEvent(breakcode, _status | 0b00000000));
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
		pushBuffer(MakeVirtualKeyEvent(make, _status | 0b00000001));
		clearCodes();
		updateStatus(MakeVirtualKeyEvent(make, _status | 0b00000001));
	} else
	{
		VirtualKeycode breakcode = convertBreakScancodeToKeycode();
		if (breakcode != VirtualKeycode::INVALID) {
			pushBuffer(MakeVirtualKeyEvent(breakcode, _status | 0b00000000));
			clearCodes();
			updateStatus(MakeVirtualKeyEvent(breakcode, _status | 0b00000000));
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

void KeyboardDriver::updateStatus(VirtualKeyEvent code) {
	// if shift is down, replace the shift bit with the make bit of the current code
	if (code.vkey == VirtualKeycode::LSHIFT || code.vkey == VirtualKeycode::RSHIFT) {
		_status &= 0b11111101;
		_status |= ((code.status & 0b00000001) << 1);
	}
	else if (code.vkey == VirtualKeycode::LCTRL || code.vkey == VirtualKeycode::RCTRL) {
		_status &= 0b11111011;
		_status |= ((code.status & 0b00000001) << 2);
	}
	else if (code.vkey == VirtualKeycode::LALT || code.vkey == VirtualKeycode::RALT) {
		_status &= 0b11110111;
		_status |= ((code.status & 0b00000001) << 3);
	}
}

VirtualKeyEvent KeyboardDriver::popBuffer()
{
	if (_bufferLength <= 0)
		return MakeVirtualKeyEvent(VirtualKeycode::INVALID, 0);
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
	
	if (_code1 == 0xF0 || (_code1 == 0xE0 && _code2 == 0xF0)) {
		return VirtualKeycode::INVALID;
	} else {
		VirtualKeycode k1 = scanset2_map1[_code1];
		if (k1 == VirtualKeycode::INVALID && _code2 != 0) {
			return scanset2_map2[_code2];
		}
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
	VirtualKeycode::INVALID,
	VirtualKeycode::F5,
	VirtualKeycode::F3,
	VirtualKeycode::F1,
	VirtualKeycode::F2,		// 06
	VirtualKeycode::F12,	// 07
	VirtualKeycode::INVALID,
	VirtualKeycode::F10,	// 09
	VirtualKeycode::F8,		// 0A
	VirtualKeycode::F6,
	VirtualKeycode::F4, 	// 0C
	VirtualKeycode::TAB,
	VirtualKeycode::TILDE,
	VirtualKeycode::INVALID,// 0F
	VirtualKeycode::INVALID,// 10
	VirtualKeycode::LALT,
	VirtualKeycode::LSHIFT,
	VirtualKeycode::INVALID,
	VirtualKeycode::LCTRL,
	VirtualKeycode::Q,
	VirtualKeycode::T_1,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 19
	VirtualKeycode::Z,
	VirtualKeycode::S,
	VirtualKeycode::A,		// 1C
	VirtualKeycode::W,
	VirtualKeycode::T_2,
	VirtualKeycode::INVALID,// 1F
	VirtualKeycode::INVALID,// 20
	VirtualKeycode::C,
	VirtualKeycode::X,
	VirtualKeycode::D,
	VirtualKeycode::E,
	VirtualKeycode::T_4,
	VirtualKeycode::T_3,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::SPACE,
	VirtualKeycode::V,
	VirtualKeycode::F,
	VirtualKeycode::T,
	VirtualKeycode::R,
	VirtualKeycode::T_5,
	VirtualKeycode::INVALID,// 2F
	VirtualKeycode::INVALID,// 30
	VirtualKeycode::N,
	VirtualKeycode::B,
	VirtualKeycode::H,
	VirtualKeycode::G,
	VirtualKeycode::Y,
	VirtualKeycode::T_6,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::M,
	VirtualKeycode::J,
	VirtualKeycode::U,
	VirtualKeycode::T_7,
	VirtualKeycode::T_8,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 40
	VirtualKeycode::COMMA,
	VirtualKeycode::K,
	VirtualKeycode::I,
	VirtualKeycode::O,
	VirtualKeycode::T_0,
	VirtualKeycode::T_9,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::DOT,
	VirtualKeycode::SLASH,
	VirtualKeycode::L,
	VirtualKeycode::COLON,
	VirtualKeycode::P,
	VirtualKeycode::T_10,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 50
	VirtualKeycode::INVALID,
	VirtualKeycode::QUOTE,
	VirtualKeycode::INVALID,
	VirtualKeycode::OPEN_SQUARE,
	VirtualKeycode::T_11,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::CAPS_LOCK,
	VirtualKeycode::RSHIFT,
	VirtualKeycode::ENTER,
	VirtualKeycode::CLOSE_SQUARE,
	VirtualKeycode::INVALID,
	VirtualKeycode::BACKSLASH,
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
	VirtualKeycode::ESCAPE,
	VirtualKeycode::NUM_LOCK,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::N_MINUS,
	VirtualKeycode::INVALID,
	VirtualKeycode::SCROLL_LOCK,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 80
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
	// Not long enough for certain keys, such as F7
	// Add entries as necessary
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
	VirtualKeycode::RALT,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::RCTRL,
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
	VirtualKeycode::N_SLASH,// 4A
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,
	VirtualKeycode::INVALID,// 50
	VirtualKeycode::INVALID,// 51
	VirtualKeycode::INVALID,// 52
	VirtualKeycode::INVALID,// 53
	VirtualKeycode::INVALID,// 54
	VirtualKeycode::INVALID,// 55
	VirtualKeycode::INVALID,// 56
	VirtualKeycode::INVALID,// 57
	VirtualKeycode::INVALID,// 58
	VirtualKeycode::INVALID,// 59
	VirtualKeycode::N_ENTER,// 5A
	VirtualKeycode::INVALID,// 5B
	VirtualKeycode::INVALID,// 5C
	VirtualKeycode::INVALID,// 5D
	VirtualKeycode::INVALID,// 5E
	VirtualKeycode::INVALID,// 5F
	VirtualKeycode::INVALID,// 60
	VirtualKeycode::INVALID,// 61
	VirtualKeycode::INVALID,// 62
	VirtualKeycode::INVALID,// 63
	VirtualKeycode::INVALID,// 64
	VirtualKeycode::INVALID,// 65
	VirtualKeycode::INVALID,// 66
	VirtualKeycode::INVALID,// 67
	VirtualKeycode::INVALID,// 68
	VirtualKeycode::END,// 69
	VirtualKeycode::INVALID,// 6A
	VirtualKeycode::L_ARROW,// 6B
	VirtualKeycode::HOME,// 6C
	VirtualKeycode::INVALID,// 6D
	VirtualKeycode::INVALID,// 6E
	VirtualKeycode::INVALID,// 6F
	VirtualKeycode::INSERT,// 70
	VirtualKeycode::DELETE,// 71
	VirtualKeycode::D_ARROW,// 72
	VirtualKeycode::INVALID,// 73
	VirtualKeycode::R_ARROW,// 74
	VirtualKeycode::U_ARROW,// 75
	VirtualKeycode::INVALID,// 76
	VirtualKeycode::INVALID,// 78
	VirtualKeycode::INVALID,// 79
	VirtualKeycode::PAGE_DOWN,// 7A
	VirtualKeycode::INVALID,// 7B
	VirtualKeycode::INVALID,// 7C
	VirtualKeycode::PAGE_UP,// 7D
	VirtualKeycode::INVALID,// 7E
	VirtualKeycode::INVALID,// 7F
	VirtualKeycode::INVALID,// 80
};

}
