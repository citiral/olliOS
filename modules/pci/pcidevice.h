#ifndef OLLIOS_PCIDEVICE_H
#define OLLIOS_PCIDEVICE_H

#include "kstd/string.h"
#include "types.h"
#include "fs/bindings.h"

class PCIDevice
{
public:
	PCIDevice(bindings::OwnedBinding* root, u8 bus, u8 dev, u8 func);
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

	void writeIOBAR(int bar, u32 value); // Write an IO BAR. Note that it will mask the return value correctly so you won't have to ;)
	u32 readIOBAR(int bar); // Read an IO BAR. Note that it will mask the return value correctly so you won't have to ;)
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
	bindings::OwnedBinding* binding;
};

#endif