#include "ata.h"
#include "atadevice.h"
#include "atachannel.h"
#include "virtualfile.h"
#include "interfacefile.h"
#include "cpu/io.h"
#include "types.h"
#include "string.h"

#define BIT_LBA48 (1<<10)

namespace ata {

AtaDevice::AtaDevice(fs::File* ata, AtaChannel* channel, AtaDrive drive, unsigned short* identify_data): drive(drive), channel(channel), _identify_data(identify_data), file(ata)
{
	readName();

	char name[32];
	sprintf(name, "ata%x", drive);

    file = new fs::VirtualFolder(name);

	file->bind(fs::InterfaceFile::read_only_string("name", _name.c_str()));
	ata->bind(file);

	fs::root->get("dev")->bind(new ATADeviceFile(this, std::string("ata")));

	u32 B = _identify_data[60] & 0xFF;
	u32 A = (_identify_data[60] >> 8) & 0xFF;
	u32 D = _identify_data[61] & 0xFF;
	u32 C = (_identify_data[61] >> 8) & 0xFF;

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
	delete[] _identify_data;
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
		name[i] = _identify_data[(i/2)+27] >> ((i&1)*8);
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
	return (_identify_data[83] & BIT_LBA48) > 0;
}

u64 AtaDevice::getBlocks()
{
	return _lba28size;
}

u64 AtaDevice::getBytes()
{
	return _lba28size * (u64) _bytesPerSector;
}

ATADeviceFileHandle::ATADeviceFileHandle(ata::AtaDevice* ata): _ata(ata)
{
}

i32 ATADeviceFileHandle::write(const void* buffer, size_t size, size_t pos)
{
	return -1;
}

i32 ATADeviceFileHandle::read(void* buffer, size_t size, size_t pos)
{
	return _ata->read(buffer, size, pos);
}

size_t ATADeviceFileHandle::get_size()
{
	return _ata->getBytes();
}

fs::File* ATADeviceFileHandle::next_child()
{
	return nullptr;
}

void ATADeviceFileHandle::reset_child_iterator()
{
}

ATADeviceFile::ATADeviceFile(ata::AtaDevice* ata, std::string&& name): _ata(ata), _name(name)
{
};

fs::FileHandle* ATADeviceFile::open()
{
	return new ATADeviceFileHandle(_ata);
}

const char* ATADeviceFile::get_name()
{
	return _name.c_str();
}

fs::File* ATADeviceFile::create(const char* name, u32 flags)
{
	return nullptr;
}

fs::File* ATADeviceFile::bind(fs::File* child)
{
	return nullptr;
}

}