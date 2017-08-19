#include "pci.h"
#include "pcidevice.h"
#include "io.h"
#include "cdefs.h"
#include "kstd/vector.h"

const u32 PCI_ENABLE_BIT = (1<<31);
const u32 PCI_CONFIG_ADDRESS = 0xCF8;
const u32 PCI_CONFIG_DATA = 0xCFC;

struct pci_device_name {
	u16 vendorId;
	u16 deviceId;
	const char* name;
};

pci_device_name pci_device_names[] = {
	{0x8086, 0x100E, "Intel Pro 1000/MT"},
	{0x8086, 0x1237, "Intel 82440LX/EX"},
	{0x8086, 0x7000, "Intel 82371SB PIIX3 PCI-to-ISA Bridge"},
	{0x8086, 0x7010, "Intel 82371SB PIIX3 IDE Interface"},
	{0x8086, 0x7020, "Intel 82371SB PIIX3 USB Host"},
	{0x8086, 0x7113, "Intel 82371?B PIIX4 Power Management"}
};

const char* unknown_device = "?";

namespace PCI
{
	const char* getName(u16 vendorId, u16 deviceId)
	{
		size_t numElement = sizeof(pci_device_names) / sizeof(pci_device_name);
		for (size_t i = 0; i < numElement; i++)
		{
			const pci_device_name* device_name = &pci_device_names[i];
			if (device_name->vendorId == vendorId && device_name->deviceId == deviceId)
			{
				return device_name->name;
			}
		}

		return unknown_device;
	}

	u32 configReadLong(u8 bus, u8 dev, u8 func, u8 reg)
	{
		u32 lbus = (u32) bus;
		u32 ldev = (u32) dev;
		u32 lfunc = (u32) func;
		u32 lreg = (u32) reg;

		u32 address = PCI_ENABLE_BIT | (lbus << 16) | (ldev << 11) | (lfunc << 8) | (lreg & 0b11111100);

		outl(PCI_CONFIG_ADDRESS, address);
		u32 val = inl(PCI_CONFIG_DATA);
		return val;
	}
	
	u16 configReadWord(u8 bus, u8 dev, u8 func, u8 reg)
	{
		u32 fval = configReadLong(bus, dev, func, reg);
		reg &= 2;
		u16 val = (fval) >> (reg*8);
		return val;
	}

	u8 configReadByte(u8 bus, u8 dev, u8 func, u8 reg)
	{
		u32 fval = configReadLong(bus, dev, func, reg);
		reg &= 3;
		u8 val = (fval) >> (reg*8);
		return val;
	}

	bool hasDevice(u8 bus, u8 dev, u8 func)
	{
		u16 vendor = configReadWord(bus, dev, func, 0);
		return vendor != 0xFFFF;
	}

	void init()
	{
		LOG_STARTUP("Searching for PCI devices...");
		std::vector<PCIDevice*> devices;

		for (u8 bus = 0; bus < 128; bus++)
		{
			for (u8 dev = 0; dev < 32; dev++)
			{
				for (u8 func = 0; func < 8; func++)
				{
					if (hasDevice(bus, dev, func))
					{
						PCIDevice* device = new PCIDevice(bus, dev, func);
						devices.push_back(device);
					}
				}
			}
		}
	}
}