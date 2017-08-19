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
	CPU::panic();
	return 0;
};

size_t PCIDevice::write(const void* data)  {
	CPU::panic();
	return 0;
};

size_t PCIDevice::write(char data) {
	CPU::panic();
	return 0;
};

size_t PCIDevice::read(void* data, size_t amount) {
	CPU::panic();
	return 0;
};

size_t PCIDevice::seek(i32 offset, int position) {
	CPU::panic();
	return 0;
};