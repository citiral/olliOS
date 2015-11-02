#include "interrupt.h"
#include "types.h"

//the IDT that is used by the operating system
static Idt idt;

IdtDescriptor::IdtDescriptor():
	offsetLow(0),
	selector(0),
	zero(0),
	typeAttr(0),
	offsetHi(0) {
}

IdtDescriptor::IdtDescriptor(u32 lower, u32 higher) {
	setValues(lower, higher);
}

void IdtDescriptor::setValues(u32 lower, u32 higher) {
	offsetLow = (u16)lower;
	selector = (u16)(lower >> 16);
	zero = (u8)higher;
	typeAttr = (u8)(higher >> 8);
	offsetHi = (u16)(higher >> 16);
}

u32 Idt::getLength() {
	return _length;
}

void Idt::setLength(u32 length) {
	_length = length;
}

void Idt::setFunction(u32 index, InterruptCallback function) {
	_callbacks[index] = function;
}

void Idt::callFunction(u32 index) {
	_callbacks[index](index);
}

void Idt::setEntry(u32 index, IdtDescriptor entry) {
	_table[index] = entry;
}

void Idt::addEntry(IdtDescriptor entry) {
	_table[_length] = entry;
	_length++;
}

IdtDescriptor Idt::getEntry(u32 index) {
	return _table[index];
}


void IdtcreateEmpty()
{
	idt = Idt();
	for (int i = 0 ; i < MAX_IDT_ENTRIES ; i++) {
		idt.addEntry(IdtDescriptor(0x00080000, 0x00008E00));
	}
}

void IdtFlush()
{
		DescriptorTablePointer pointer;
		pointer.base = (u32)&idt;
		pointer.limit = (u16)(idt.getLength()*8 - 1);
		reload_idt(pointer.limit, pointer.base);
}