#ifndef OLLIOS_ATADEVICE_H
#define OLLIOS_ATADEVICE_H

#include "devices/blockdevice.h"
#include "cdefs.h"
#include "kstd/string.h"

class AtaDevice: public BlockDevice {
public:
    AtaDevice(u16 port, unsigned short* data, int drive);
    ~AtaDevice();

	std::string& getName();
	bool supportsLBA48();
	int getDrive(); // Check what drive this is. 0 = master, 1 = false

	u64 getBlocks();
	u64 getBytes();

	void selectDevice();

    virtual DeviceType getDeviceType() const =0;
    virtual void getDeviceInfo(void* deviceinfo) const =0;

	virtual size_t write(const void* data, size_t amount) =0;
	virtual size_t write(const void* data) =0;
	virtual size_t write(char data) =0;
	virtual size_t read(void* data, size_t amount) =0;
	virtual size_t seek(i32 offset, int position) =0;

	void outB(u16 reg, u8 value);
	void outW(u16 reg, u16 value);
	void outL(u16 reg, u32 value);
	void outSW (unsigned short int reg, const void *addr, unsigned long int count);
	
	u8 inB(u16 reg);
	u16 inW(u16 reg);
	u32 inL(u16 reg);
	void inSW (unsigned short int reg, void *addr, unsigned long int count);

protected:
	unsigned short* _data;
	size_t _bytesPerSector = 512;

private:
	void readName();
	/*void writeRegisterB(u8 reg, u8 data);
	void writeRegisterW(u8 reg, u16 data);
	u8 readRegisterB(u8 reg);
	u16 readRegisterW(u8 reg);*/

	u16 _port;
	std::string _name;
    u8 _drive;
    u32 _lba28size;
};

#endif