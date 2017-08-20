#include "pcidevice.h"
#include "pci.h"
#include "cpu.h"
#include "cdefs.h"
#include "string.h"

PCIDevice::PCIDevice(u8 bus, u8 dev, u8 func)
{
	_bus = bus;
	_device = dev;
	_func = func;

	_vendorId = configReadWord(0);
	_deviceId = configReadWord(2);
	_revisionId = configReadByte(8);

	_deviceName = PCI::getName(_vendorId, _deviceId);

	_classCode = configReadByte(11);
	_subclassCode = configReadByte(10);
	_headerType = configReadByte(14);

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
	/*DevicePCIInfo* info = (DevicePCIInfo*) deviceinfo;
	info->deviceInfo.name = _deviceName;*/
	DeviceKeyboardInfo* info = (DeviceKeyboardInfo*)deviceinfo;
	info->deviceInfo.name = "gd";
}

size_t PCIDevice::write(const void* data, size_t amount) {
	CPU::panic("Call to unimplemented function write in PCIDevice.cpp");
	UNUSED(data);
	UNUSED(amount);
	return 0;
};

size_t PCIDevice::write(const void* data)  {
	CPU::panic("Call to unimplemented function write in PCIDevice.cpp");
	CPU::panic();
	UNUSED(data);
	return 0;
};

size_t PCIDevice::write(char data) {
	CPU::panic("Call to unimplemented function write in PCIDevice.cpp");
	CPU::panic();
	UNUSED(data);
	return 0;
};

size_t PCIDevice::read(void* data, size_t amount) {
	CPU::panic("Call to unimplemented function read in PCIDevice.cpp");
	CPU::panic();
	UNUSED(data);
	UNUSED(amount);
	return 0;
};

size_t PCIDevice::seek(i32 offset, int position) {
	CPU::panic("Call to unimplemented function seek in PCIDevice.cpp");
	CPU::panic();
	UNUSED(offset);
	UNUSED(position);
	return 0;
};