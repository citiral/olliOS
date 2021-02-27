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

GdtDescriptor::GdtDescriptor(u32 limit, u32 base, u8 access, u8 flags) {
	this->limit = (u16) limit;
	this->limitFlags = (u8) (limit >> 16);
	this->limitFlags |= (flags & 0xF0);

	this->baseLow = (u16) base;
	this->baseMid = (u8) (base >> 16);
	this->baseHi = (u8) (base >> 24);

	this->access = access;
}

void GdtCreateFlat() {
	GDTTable[0] = GdtDescriptor(0x00000000, 0x00000000);//NULL entry: 0x00
	GDTTable[1] = GdtDescriptor(0x0000FFFF, 0x00CF9A00);//PL0 Code    0x8
	GDTTable[2] = GdtDescriptor(0x0000FFFF, 0x00CF9200);//PL0 Data    0x10
	GDTTable[3] = GdtDescriptor(0x0000FFFF, 0x00CFFA00);//PL3 Code    0x18
	GDTTable[4] = GdtDescriptor(0x0000FFFF, 0x00CFF200);//PL3 Data    0x20
	//GDTTable[5] = GdtDescriptor(0x00000068, 0x00408900);//TSS entry   0x28
	GDTSize = 5*8;
}

void GdtFlush() {
	reloadGdt(GDTSize, (u32)GDTTable - 0xC0000000);
}

u16 GdtSize() {
	return GDTSize;
}

u32 GdtOffset() {
	return (u32)GDTTable - 0xC0000000;
}

u32 GdtAddTss(tss::TaskStateSegment* tss) {
	u32 index = GDTSize / 8;

	GDTTable[index] = GdtDescriptor(sizeof(tss::TaskStateSegment), (u32) tss, 0x89, 0x40);
	GDTSize += 8;

	return index * 8;
}