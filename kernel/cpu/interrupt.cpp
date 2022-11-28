#include "interrupt.h"
#include "types.h"
#include "interruptHandlers.h"
#include "pic.h"
#include "stdio.h"
#include "apic.h"
#include "cdefs.h"

extern "C" void isr_wrapper();
extern "C" void intHandlerGeneralProtectionViolation_asm();
extern "C" void intHandlerPageFault_asm();
extern "C" void intHandlerDummyKeyboard_asm();

//the IDT that is used by the operating system
Idt idt;

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

void IdtDescriptor::setOffset(u32 offset) {
	offsetHi = (u16)(offset >> 16);
	offsetLow = (u16)offset;
}

u32 Idt::getLength() {
	return _length;
}

void Idt::setLength(u32 length) {
	_length = length;
}

void Idt::setEntry(u32 index, IdtDescriptor entry) {
	_table[index] = entry;
}

void Idt::addEntry(IdtDescriptor entry) {
	_table[_length] = entry;
	_length++;
}

IdtDescriptor& Idt::getEntry(u32 index) {
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

u32 IdtBase()
{
	return (u32)&idt;
}

u16 IdtLimit()
{
	return (u16)(idt.getLength()*8 - 1);
}

extern "C" void c_interrupt_wrapper() {
	//CPU::panic("unknown interrupt!\n");
	end_interrupt(0x20);
}

extern "C" void end_interrupt(u32 irq) {
	if (apic::enabled()) {
        apic::endInterrupt(irq);
    } else {
        endInterrupt(irq - 0x20);
    }
}

extern "C" void __attribute__ ((noinline)) IdtRegisterInterrupts()
{
	for (u32 x = 0; x < MAX_IDT_ENTRIES; x++)
		idt.getEntry(x).setOffset((u32) &isr_wrapper);
	
	idt.getEntry(INT_GENERAL_PROTECTION_VIOLATION).setOffset((u32) &intHandlerGeneralProtectionViolation_asm);
    idt.getEntry(INT_PAGE_FAULT).setOffset((u32) &intHandlerPageFault_asm);
    idt.getEntry(INT_KEYBOARD).setOffset((u32) &intHandlerDummyKeyboard_asm);
}
