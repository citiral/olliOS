#include "ata/atadevice.h"
#include "io.h"

#define BIT_LBA48 (1<<10)

AtaDevice::AtaDevice(u16 port, unsigned short* data, int drive) : _port(port), _data(data), _drive(drive)
{
	readName();
}

AtaDevice::~AtaDevice()
{}

std::string& AtaDevice::getName()
{
	return _name;
}

void AtaDevice::readName()
{
	size_t lastCharPos = 0;
	char name[40];

	for (int i = 0; i < 40; i++)
	{
		name[i] = _data[(i/2)+27] >> ((i&1)*8);
		if (name[i] != ' ' && name[i] != '\0')
			lastCharPos = i;
	}
	
	_name = std::string(name, lastCharPos+1);
}

bool AtaDevice::supportLBA48()
{
	return _data[83] & BIT_LBA48 > 0;
}

//
// IO functions implemented here
// 
void AtaDevice::outB(u16 reg, u8 value) {
	outb(_port+reg, value);
}

void AtaDevice::outW(u16 reg, u16 value) {
	outw(_port+reg, value);
}

void AtaDevice::outL(u16 reg, u32 value) {
	outL(_port+reg, value);
}

void AtaDevice::outSW(unsigned short int reg, const void *addr, unsigned long int count) {
	outsw(_port+reg, addr, count);
}

u8 AtaDevice::inB(u16 reg) {
	return inb(_port+reg);
}

u16 AtaDevice::inW(u16 reg) {
	return inw(_port+reg);
}

u32 AtaDevice::inL(u16 reg) {
	return inl(_port+reg);
}

void AtaDevice::inSW(unsigned short int reg, void *addr, unsigned long int count) {
	insw(_port+reg, addr, count);
}