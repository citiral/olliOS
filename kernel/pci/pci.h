#ifndef OLLIOS_PCI_H
#define OLLIOS_PCI_H

#include "types.h"
#include "kstd/string.h"

namespace PCI
{
	const char* getName(u16 vendorId, u16 devId);

	u32 configReadLong(u8 bus, u8 dev, u8 func, u8 reg);
	u16 configReadWord(u8 bus, u8 dev, u8 func, u8 reg);
	u8 configReadByte(u8 bus, u8 dev, u8 func, u8 reg);

	void init();
}

#endif