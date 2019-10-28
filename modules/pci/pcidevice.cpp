#include "pcidevice.h"
#include "pci.h"
#include "cpu.h"
#include "cdefs.h"
#include "string.h"

PCIDevice::PCIDevice(u8 bus, u8 dev, u8 func)
{
	// Save these values
	_bus = bus;
	_device = dev;
	_func = func;

	// Read some basic values from the device
	_vendorId = configReadWord(0);
	_deviceId = configReadWord(2);
	_revisionId = configReadByte(8);

	// Get the device name from the hard coded list (no other way, sorry)
	_deviceName = PCI::getName(_vendorId, _deviceId);

	// Get the device class
	_classCode = configReadByte(11);
	_subclassCode = configReadByte(10);
	_headerType = configReadByte(14);

	// Print the name to the console
	if (_deviceName[0] == '?')
		printf("PCI%d:%d.%d, VendorID=0x%X, DevID=0x%X, Rev=0x%X: Class=0x%X:0x%X 0x%X\n", bus, dev, func, _vendorId, _deviceId, _revisionId, _classCode, _subclassCode, _headerType);
	else
		printf("PCI%d:%d.%d, %s, Rev=0x%X: Class=0x%X:0x%X 0x%X\n", bus, dev, func, _deviceName, _revisionId, _classCode, _subclassCode, _headerType);
}

PCIDevice::~PCIDevice()
{

}

u32 PCIDevice::configReadLong(u8 reg)
{
	return PCI::configReadLong(_bus, _device, _func, reg);
}

u16 PCIDevice::configReadWord(u8 reg)
{
	return PCI::configReadWord(_bus, _device, _func, reg);
}

u8 PCIDevice::configReadByte(u8 reg)
{
	return PCI::configReadByte(_bus, _device, _func, reg);
}

void PCIDevice::configWriteLong(u8 reg, u32 value)
{
	PCI::configWriteLong(_bus, _device, _func, reg, value);
}

void PCIDevice::configWriteWord(u8 reg, u16 value)
{
	PCI::configWriteWord(_bus, _device, _func, reg, value);
}

void PCIDevice::configWriteByte(u8 reg, u8 value)
{
	PCI::configWriteByte(_bus, _device, _func, reg, value);
}

DeviceType PCIDevice::getDeviceType() const
{
	return DeviceType::PCI;
}

void PCIDevice::getDeviceInfo(void* deviceinfo) const
{
	DevicePCIInfo* info = (DevicePCIInfo*) deviceinfo;
	info->deviceInfo.name = _deviceName;
}

void PCIDevice::writeIOBAR(int bar, u32 value)
{
	u8 location = 0x10 + 4*bar;
	value = (value & ~(2)) | 1;
	configWriteLong(location, value);
}

u32 PCIDevice::readIOBAR(int bar)
{
	u8 location = 0x10 + 4*bar;
	u32 value = configReadLong(location);
	value &= 0xFFFFFFFC;
	return value;
}
