#include "serial.h"
#include "devicemanager.h"
#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "io.h"

#define SERIAL_BDA_BASE 0x0400

#define TRANSMIT_BUFFER 0x0
#define RECEIVE_BUFFER 0x0
#define DL_LOW 0x0
#define DL_HIGH 0x1
#define INT_EN_REG 0x1
#define INT_ID_REG 0x2
#define FIFO_CONTROL_REG 0x2
#define LINE_CONTROL_REG 0x3
#define MODEM_CONTROL_REG 0x4
#define LINE_STATUS_REG 0x5
#define MODEM_STATUS_REG 0x6
#define SCRATCH_REG 0x7

Serial::Serial(u16 port)
{
	_port = port;
	LOG_STARTUP("Serial device at 0x%X", port);
}

Serial::~Serial()
{

}

DeviceType Serial::getDeviceType() const
{
	return DeviceType::Serial;
}

void Serial::getDeviceInfo(void* deviceinfo) const
{
	DeviceSerialInfo* info = (DeviceSerialInfo*) deviceinfo;
	info->deviceInfo.name = "SERIAL";
}

size_t Serial::write(const void* data, size_t amount)
{
	const char* str = (const char*) data;
	for (size_t i = 0; i < amount; i++)
		write((char) str[i]);
	return 0;
}

size_t Serial::write(const void* data)
{
	const char* str = (const char*) data;
	write(data, strlen(str));
	return 0;
}

size_t Serial::write(char data)
{
	outb(_port, data);
	return 0;
}

size_t Serial::read(void* data, size_t amount)
{
	UNUSED(data);
	UNUSED(amount);
	CPU::panic("read not implemented in serial.cpp");
	return 0;
}

size_t Serial::seek(i32 offset, int position)
{
	UNUSED(offset);
	UNUSED(position);
	CPU::panic("seek not implemented in serial.cpp");
	return 0;
}

void Serial::writeRegister(u8 reg, u8 val)
{
	outb(_port + reg, val);
}

u8 Serial::readRegister(u8 reg)
{
	return inb(_port + reg);
}

void initSerialDevices()
{
	for (size_t i = 0; i < 8; i+=2)
	{
		u16 port = *((u16*) (SERIAL_BDA_BASE+i));
		if (port != 0)
		{
			Serial* serial = new Serial(port);
			deviceManager.addDevice(serial);
		}
	}
}