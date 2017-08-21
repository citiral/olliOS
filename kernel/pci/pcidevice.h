#ifndef OLLIOS_PCIDEVICE_H
#define OLLIOS_PCIDEVICE_H

#include "devices/device.h"
#include "kstd/string.h"
#include "types.h"

class PCIDevice : public Device
{
public:
	PCIDevice(u8 bus, u8 dev, u8 func);
	~PCIDevice();
	u16 getVendorID();
	u16 getFunction();
	u16 getDeviceID();
	u16 getRevision();

	u32 configReadLong(u8 reg);
	u16 configReadWord(u8 reg);
	u8 configReadByte(u8 reg);

	void configWriteLong(u8 reg, u32 value);
	void configWriteWord(u8 reg, u16 value);
	void configWriteByte(u8 reg, u8 value);

    DeviceType getDeviceType() const;
	void getDeviceInfo(void* deviceinfo) const;

    size_t write(const void* data, size_t amount);
    size_t write(const void* data);
    size_t write(char data);
    size_t read(void* data, size_t amount);
    size_t seek(i32 offset, int position);
private:
	u8 _bus;
	u8 _device;
	u8 _func;

	u16 _vendorId;
	u16 _deviceId;
	u16 _revisionId;

	u8 _classCode;
	u8 _subclassCode;
	u8 _headerType;

	const char* _deviceName;
};

#endif