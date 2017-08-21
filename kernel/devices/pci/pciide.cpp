#include "devices/pci/pciide.h"

PCIIDE::PCIIDE(u8 bus, u8 dev, u8 func) : PCIDevice(bus, dev, func) {
}

PCIIDE::~PCIIDE() {

}

u32 PCIIDE::getBaseAddress(int drive) {
	u32 address = readIOBAR(drive*2);
	if (address == 0)
		return (drive==0)?0x1F0:0x170;
	else
		return address;
}

u32 PCIIDE::getControlBaseAddress(int drive) {
	u32 address = readIOBAR(1+drive*2);
	if (address == 0)
		return (drive==1)?0x3F6:0x376;
	else
		return address;
}