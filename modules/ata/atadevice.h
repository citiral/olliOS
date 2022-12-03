#ifndef OLLIOS_ATADEVICE_H
#define OLLIOS_ATADEVICE_H

#include "filesystem/file.h"
#include "cdefs.h"
#include "kstd/string.h"
#include "ata.h"
#include "atachannel.h"

namespace ata {

class AtaDevice {
public:
    AtaDevice(fs::File* ata, AtaChannel* channel, AtaDrive drive, unsigned short* identify_data, u32 deviceId);
    ~AtaDevice();

	std::string& getName();
	bool supportsLBA48();

	u64 getBlocks();
	u64 getBytes();

    virtual size_t write(const void* data, size_t amount, size_t offset) {return 0;};
    virtual size_t read(void* data, size_t amount, size_t offset) {return 0;};

protected:
	AtaDrive drive;
	AtaChannel* channel;

	unsigned short* _identify_data;
	size_t _bytesPerSector = 512;
	fs::File* file;

private:
	void readName();

	std::string _name;
    u32 _lba28size;
};



class ATADeviceFileHandle: public fs::FileHandle {
public:
	ATADeviceFileHandle(ata::AtaDevice* ata);

	i32 write(const void* buffer, size_t size, size_t pos);
	i32 read(void* buffer, size_t size, size_t pos);

	size_t get_size();
	fs::File* next_child();
	void reset_child_iterator();

private:
	ata::AtaDevice* _ata;
};


class ATADeviceFile: public fs::File {
public:
	ATADeviceFile(ata::AtaDevice* ata, std::string&& name);

	fs::FileHandle* open();
	const char* get_name();
	File* create(const char* name, u32 flags);
	File* bind(File* child);

public:
	std::string _name;
	ata::AtaDevice* _ata;
};


}

#endif