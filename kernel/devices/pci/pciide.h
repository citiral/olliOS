#ifndef OLLIOS_PCIIDE_H
#define OLLIOS_PCIIDE_H

#include "devices/pci/pcidevice.h"
#include "types.h"

class PCIIDE : public PCIDevice {
public:
	PCIIDE(u8 bus, u8 dev, u8 func);
	~PCIIDE();

	u32 getBaseAddress(int drive);
	u32 getControlBaseAddress(int drive);

	u32 getBusMasterBaseAddress();
};

#endif