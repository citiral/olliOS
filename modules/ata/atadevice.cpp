#include "file.h"
#include "virtualfile.h"
#include "interfacefile.h"
#include "atadevice.h"
#include "ata.h"
#include "cpu/io.h"
#include "types.h"
#include "string.h"

#define BIT_LBA48 (1<<10)

class ATADeviceFileHandle: public fs::FileHandle {
public:
	ATADeviceFileHandle(ata::AtaDevice* ata): _ata(ata) {

	}

	i32 write(const void* buffer, size_t size, size_t pos) {
		return -1;
	}

	i32 read(void* buffer, size_t size, size_t pos) {
		return _ata->read(buffer, size, pos);
	}

	size_t get_size() {
		return _ata->getBytes();
	}

	fs::File* next_child() {
		return nullptr;
	}

	void reset_child_iterator() {

	}

private:
	ata::AtaDevice* _ata;
};

class ATADeviceFile: public fs::File {
public:
	ATADeviceFile(ata::AtaDevice* ata, std::string&& name): _ata(ata), _name(name) {

	};

	fs::FileHandle* open() {
		return new ATADeviceFileHandle(_ata);
	}

	const char* get_name() {
		return _name.c_str();
	}

	File* create(const char* name, u32 flags) {
		return nullptr;
	}

	File* bind(File* child) {
		return nullptr;
	}

public:
	std::string _name;
	ata::AtaDevice* _ata;
};

namespace ata {

AtaDevice::AtaDevice(fs::File* ata, u16 port, unsigned short* data, int drive) : _data(data), _port(port),_drive(drive)
{
	readName();

	char name[32];
	sprintf(name, "ata%x", drive);

    file = new fs::VirtualFolder(name);

	file->bind(fs::InterfaceFile::read_only_string("name", _name.c_str()));
	ata->bind(file);

	fs::root->get("dev")->bind(new ATADeviceFile(this, std::string("ata")));

	/*bindings::root->get("dev")->add((new ATABinding(std::string("ata") + ('0' + drive), this))->on_read([](bindings::OwnedBinding* _binding, void* buffer, size_t size, size_t offset) {
		ATABinding* binding = (ATABinding*) _binding;
		binding->ata->seek(offset, SEEK_SET);
		return binding->ata->read(buffer, size, offset);
	}));*/


	u32 B = data[60] & 0xFF;
	u32 A = (data[60] >> 8) & 0xFF;
	u32 D = data[61] & 0xFF;
	u32 C = (data[61] >> 8) & 0xFF;

	_lba28size = (A << 0) | (B << 8) | (C << 16) | (D << 24);

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
	
	for (size_t i = 0; i < lastCharPos; i++) {
		if (name[i] == ' ')
			name[i] = '_';
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

u64 AtaDevice::getBlocks()
{
	return _lba28size;
}

u64 AtaDevice::getBytes()
{
	return _lba28size * (u64) _bytesPerSector;
}

void AtaDevice::selectDevice()
{
	driver.selectDevice(_port, _drive);
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

}
