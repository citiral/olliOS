#include "pcidevice.h"
#include "pci.h"
#include "cpu/cpu.h"
#include "cdefs.h"
#include "string.h"
#include "stdio.h"
#include "filesystem/virtualfolder.h"
#include "filesystem/interfacefile.h"

template<size_t BAR>
int read_pci_bar(char* buffer, size_t length, size_t pos, void* context) {
	UNUSED(pos);
	PCIDevice* device = (PCIDevice*) context;
	u32 result;

	if (length < sizeof(result)) {
		return 0;
	}

	result = device->readIOBAR(BAR);
	memcpy(buffer, &result, sizeof(result));

	return sizeof(result);

}

PCIDevice::PCIDevice(fs::File* root, u8 bus, u8 dev, u8 func)
{
	// Save these values
	_bus = bus;
	_device = dev;
	_func = func;

	// Read some basic values from the device
	_vendorId = configReadWord(0);
	_deviceId = configReadWord(2);
	_revisionId = configReadByte(8);
	_interruptPin = configReadByte(0x3D);

	// Get the device name from the hard coded list (no other way, sorry)
	_deviceName = PCI::getName(_vendorId, _deviceId);

	// Get the device class
	_programmingInterface = configReadByte(9);
	_classCode = configReadByte(11);
	_subclassCode = configReadByte(10);
	_headerType = configReadByte(14);
	_headerType = configReadByte(14);

	char name[32];
	sprintf(name, "%d:%d:%d", _bus, _device, _func);

	_file = new fs::VirtualFolder(name);

	_file->bind(fs::InterfaceFile::read_only<sizeof(_bus)>("bus", &_bus));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_device)>("device", &_device));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_func)>("func", &_func));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_vendorId)>("vendor", &_vendorId));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_deviceId)>("device", &_deviceId));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_revisionId)>("revision", &_revisionId));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_classCode)>("class", &_classCode));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_subclassCode)>("subclass", &_subclassCode));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_headerType)>("header_type", &_headerType));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_programmingInterface)>("interface", &_programmingInterface));
	_file->bind(fs::InterfaceFile::read_only<sizeof(_programmingInterface)>("pin", &_interruptPin));

	_file->bind(new fs::InterfaceFile("name", nullptr, [](char* buffer, size_t length, size_t pos, void* context) {
		UNUSED(pos);
		const char* name = (const char*) context;

		if (length == 0) {
			return 0;
		}

		size_t name_length = strlen(name);
		size_t count = length > name_length ? name_length : length - 1;

		memcpy(buffer, context, count);
		buffer[count] = 0;
		return (int)count + 1;
	}, (void*) _deviceName));

	if (_headerType == 0) {
		fs::File* bars = _file->create("bars", FILE_CREATE_DIR);
		bars->bind(new fs::InterfaceFile("0", nullptr, read_pci_bar<0>, this));
		bars->bind(new fs::InterfaceFile("1", nullptr, read_pci_bar<1>, this));
		bars->bind(new fs::InterfaceFile("2", nullptr, read_pci_bar<2>, this));
		bars->bind(new fs::InterfaceFile("3", nullptr, read_pci_bar<3>, this));
		bars->bind(new fs::InterfaceFile("4", nullptr, read_pci_bar<4>, this));
		bars->bind(new fs::InterfaceFile("5", nullptr, read_pci_bar<5>, this));
	}
	
	root->bind(_file);

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
