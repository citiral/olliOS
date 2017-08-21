#ifndef OLLIOS_SERIAL_H
#define OLLIOS_SERIAL_H

#include "devices/blockdevice.h"

// Serial speeds
// The number of the clock divisor used to actually set the speed.
// This table can be found at http://retired.beyondlogic.org/serial/serial.htm#12
enum class SerialSpeed : u32
{
	B50 = 2304,
	B300 = 384,
	B600 = 192,
	B2400 = 48,
	B4800 = 24,
	B9600 = 12,
	B19200 = 6,
	B38400 = 3,
	B57600 = 2,
	B115200 = 1
};

class Serial : public BlockDevice
{
public:
	Serial(u16 port);
	~Serial();

	DeviceType getDeviceType() const;
	void getDeviceInfo(void* deviceinfo) const;

    size_t write(const void* data, size_t amount);
    size_t write(const void* data);
    size_t write(char data);
    size_t read(void* data, size_t amount);
	size_t seek(i32 offset, int position);
private:
	void writeRegister(u8 reg, u8 val);
	u8 readRegister(u8 reg);

	void setDLAB(bool state);

	u16 _port;
};

void initSerialDevices();

#endif