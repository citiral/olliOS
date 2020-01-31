#include "pcidevice.h"
#include "pci.h"
#include "cpu.h"
#include "cdefs.h"
#include "string.h"
#include "stdio.h"

class PCIBinding: public bindings::OwnedBinding {
public:
	PCIBinding(std::string name, PCIDevice* device, u32 bar): bindings::OwnedBinding(name), _device(device), _bar(bar)
	{
		on_read([](OwnedBinding* _binding, void* buffer, size_t size, size_t offset) {
			PCIBinding* binding = (PCIBinding*) _binding;

			u32 result = binding->_device->readIOBAR(binding->_bar);
			if (size + offset > 4) {
				size = 4 - offset;
			}

			if (size > 0) {
				memcpy(buffer, &result, size);
			}
			
			return size;
		});
	}


private:
	PCIDevice* _device;
	u32 _bar;
};

PCIDevice::PCIDevice(bindings::OwnedBinding* root, u8 bus, u8 dev, u8 func)
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

	// Register it to the binding tree
	char name[32];
	sprintf(name, "%.2X:%.2X:%.2X", _bus, _device, _func);
	binding = new bindings::OwnedBinding(name);
	binding->add(new bindings::RefMemoryBinding("bus", &_bus, sizeof(_bus)));
	binding->add(new bindings::RefMemoryBinding("device", &_device, sizeof(_device)));
	binding->add(new bindings::RefMemoryBinding("func", &_func, sizeof(_func)));
	binding->add(new bindings::RefMemoryBinding("vendor", &_vendorId, sizeof(_vendorId)));
	binding->add(new bindings::RefMemoryBinding("device", &_deviceId, sizeof(_deviceId)));
	binding->add(new bindings::RefMemoryBinding("revision", &_revisionId, sizeof(_revisionId)));
	binding->add(new bindings::RefMemoryBinding("name", _deviceName, strlen(_deviceName)+1));
	binding->add(new bindings::RefMemoryBinding("class", &_classCode, sizeof(_classCode)));
	binding->add(new bindings::RefMemoryBinding("subclass", &_subclassCode, sizeof(_subclassCode)));
	binding->add(new bindings::RefMemoryBinding("header_type", &_headerType, sizeof(_headerType)));

	if (_headerType == 0) {
		bindings::OwnedBinding* bars = new bindings::OwnedBinding("bars");
		bars->add(new PCIBinding("0", this, 0));
		bars->add(new PCIBinding("1", this, 1));
		bars->add(new PCIBinding("2", this, 2));
		bars->add(new PCIBinding("3", this, 3));
		bars->add(new PCIBinding("4", this, 4));
		bars->add(new PCIBinding("5", this, 5));
		binding->add(bars);
	}
	
	root->add(binding);

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
