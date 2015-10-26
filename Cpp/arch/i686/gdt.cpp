#include "gdt.h"

//asm C reload gdt function
extern "C" void reloadGdt(u16 limit, u32 base);

static GdtDescriptor GDTTable[MAX_GDT_ENTRIES];
static u16 GDTSize;

GdtDescriptor::GdtDescriptor() {
	limit = 0;
	baseLow = 0;
	baseMid = 0;
	access = 0;
	limitFlags = 0;
	baseHi = 0;
}

GdtDescriptor::GdtDescriptor(u32 lower, u32 higher) {
	limit = (u16) lower;
	baseLow = (u16)(lower >> 16);
	baseMid = (u8)higher;
	access = (u8)(higher >> 8);
	limitFlags = (u8)(higher >> 16);
	baseHi = (u8)(higher >> 24);
}

void GdtCreateFlat() {
	GDTTable[0] = GdtDescriptor(0x00000000, 0x00000000);//NULL entry: 0x00
	GDTTable[1] = GdtDescriptor(0x0000FFFF, 0x00CF9A00);//PL0 Code    0x8
	GDTTable[2] = GdtDescriptor(0x0000FFFF, 0x00CF9200);//PL0 Data    0x10
	GDTTable[3] = GdtDescriptor(0x0000FFFF, 0x00CFFA00);//PL3 Code    0x18
	GDTTable[4] = GdtDescriptor(0x0000FFFF, 0x00CFF200);//PL3 Data    0x20
	GDTTable[5] = GdtDescriptor(0x00000068, 0x00408900);//TSS entry   0x28
	GDTSize = 6*8;
}

void GdtFlush() {
	reloadGdt(GDTSize, (u32)GDTTable);
}