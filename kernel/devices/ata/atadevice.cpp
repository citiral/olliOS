#include "devices/ata/atadevice.h"
#include "devices/ata/ata.h"
#include "io.h"
#include "types.h"

#define BIT_LBA48 (1<<10)

AtaDevice::AtaDevice(u16 port, unsigned short* data, int drive) : _port(port), _data(data), _drive(drive)
{
	readName();

	u32 B = data[60] & 0xFF;
	u32 A = (data[60] >> 8) & 0xFF;
	u32 D = data[61] & 0xFF;
	u32 C = (data[61] >> 8) & 0xFF;

	_lba28size = (A << 0) | (B << 8) | (C << 16) | (D << 24);

	//_lba28size = (((u32) data[60])) | ((((u32) data[61]) & 0x0FFFFFFF) << 16);
	if (_lba28size != 0)
	{
		u64 bytes = (u64) _lba28size * (u64) _bytesPerSector;
		u64 kib = bytes / 1024;
		u64 mib = kib / 1024;
		u64 gib = mib / 1024;
		printf("Disk size = %d blocks, %dKiB, %dMiB, %dGiB\n", _lba28size, (u32) kib, (u32) mib, (u64) gib);
	}
}

AtaDevice::~AtaDevice()
{
	delete[] _data;
}

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

bool AtaDevice::supportsLBA48()
{
	return (_data[83] & BIT_LBA48) > 0;
}

int AtaDevice::getDrive()
{
	return _drive;
}

void AtaDevice::selectDevice()
{
	ataDriver.selectDevice(_port, _drive);
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