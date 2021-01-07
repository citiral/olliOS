#include "pci.h"
#include "pcidevice.h"
#include "pciide.h"

#include "cpu/io.h"
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
	{0x1106, 0x3205, "VIA CPU-to-PCI Bridge"},
	{0x1106, 0xB198, "VIA PCI-to-PCI Bridge"},
	{0x1106, 0x3038, "VIA VT8251 USB1.0 Controller"},
	{0x1106, 0x3104, "VIA VT6202 USB2.0 EHCI Controller"},
	{0x1106, 0x3177, "VIA PCI-to-ISA Bridge"},
	{0x1106, 0x0571, "VT8235 Bus Master IDE Controller"},
	{0x1106, 0x3059, "VIA Sound AC97"},
	{0x1106, 0x3065, "VIA VT6102 Rhine II Fast Ethernet"},
	{0x1106, 0x7205, "VIA MK400 S3 UniChrome Graphics Adapter"},
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

	u32 formatAddress(u8 bus, u8 dev, u8 func, u8 reg)
	{
		u32 lbus = (u32) bus;
		u32 ldev = (u32) dev;
		u32 lfunc = (u32) func;
		u32 lreg = (u32) reg;

		u32 address = PCI_ENABLE_BIT | (lbus << 16) | (ldev << 11) | (lfunc << 8) | (lreg & 0b11111100);
		return address;
	}

	u32 configReadLong(u8 bus, u8 dev, u8 func, u8 reg)
	{
		u32 address = formatAddress(bus, dev, func, reg);
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

	void configWriteLong(u8 bus, u8 dev, u8 func, u8 reg, u32 value)
	{
		u32 address = formatAddress(bus, dev, func, reg);
		outl(PCI_CONFIG_ADDRESS, address);
		outl(PCI_CONFIG_DATA, value);
	}

	void configWriteWord(u8 bus, u8 dev, u8 func, u8 reg, u16 value)
	{
		u32 address = formatAddress(bus, dev, func, reg);
		outl(PCI_CONFIG_ADDRESS, address);
		outw(PCI_CONFIG_DATA, value);
	}

	void configWriteByte(u8 bus, u8 dev, u8 func, u8 reg, u8 value)
	{
		u32 address = formatAddress(bus, dev, func, reg);
		outl(PCI_CONFIG_ADDRESS, address);
		outb(PCI_CONFIG_DATA, value);
	}

	bool hasDevice(u8 bus, u8 dev, u8 func)
	{
		u16 vendor = configReadWord(bus, dev, func, 0);
		return vendor != 0xFFFF;
	}

	void init(fs::File *root)
	{
		LOG_STARTUP("Searching for PCI devices...");

		for (u8 bus = 0; bus < 128; bus++)
		{
			for (u8 dev = 0; dev < 32; dev++)
			{
				for (u8 func = 0; func < 8; func++)
				{
					if (hasDevice(bus, dev, func))
					{
						u8 classCode = configReadByte(bus, dev, func, 11);
						u8 subclassCode = configReadByte(bus, dev, func, 10);

						PCIDevice* device;
						//if (classCode == 0x1 && subclassCode == 0x1)
						//	device = new PCIIDE(bind, bus, dev, func);
						//else
						device = new PCIDevice(root, bus, dev, func);

						//deviceManager.addDevice(device);
					}
				}
			}
		}
	}
}