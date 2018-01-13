#ifndef __GDT_H
#define __GDT_H

#include "types.h"
#include "descriptor.h"

#define MAX_GDT_ENTRIES 255

//represents a high level entry for a gdtDescriptor
class __attribute__((packed)) GdtDescriptor {
public:
	GdtDescriptor();
	GdtDescriptor(u32 lower, u32 higher);

	/*void setLimit(u32 limit);
	void setBase(u32 base);
	void setPresent(bool present);
	void setDpl(Dpl dpl);
	void setExecutable(bool excecutable);
	void setDirection(bool up);
	void setRw(bool writeable);
	void setGranularity(bool page);
	void setSize(bool size);
	void setType(u8 access);*/

	u16 limit;
	u16 baseLow;
	u8 baseMid;
	u8 access;
	u8 limitFlags;
	u8 baseHi;
};

void GdtCreateFlat();
void GdtFlush();
u16 GdtSize();
u32 GdtOffset();

#endif /* end of include guard: __GDT_H */
