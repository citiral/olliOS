#pragma once

#include "descriptor.h"
#include "types.h"

#define MAX_IDT_ENTRIES 256
#define THROW_INTERRUPT(x) __asm__ volatile("int $" #x);
#define INT_KEYBOARD 0x21


typedef void (*InterruptCallback)(u32);

///A single entry in the Idt table
class IdtDescriptor {
	public:
		IdtDescriptor();
		IdtDescriptor(u32 lower, u32 higher);
		void setValues(u32 lower, u32 higher);
		void setOffset(u32 offset);
		u16 offsetLow;
		u16 selector;
		u8 zero;
		u8 typeAttr;
		u16 offsetHi;
};


class Idt {
public:
	u32 getLength();
	void setLength(u32 length);
	void setFunction(u32 index, InterruptCallback function);
	void callFunction(u32 index);
	void setEntry(u32 index, IdtDescriptor entry);
	void addEntry(IdtDescriptor entry);
	IdtDescriptor& getEntry(u32 index);


private:
	///The table containing the gdt descriptors
	IdtDescriptor _table[MAX_IDT_ENTRIES];
	///the list of callbacks linked to the interrupts
	InterruptCallback _callbacks[MAX_IDT_ENTRIES];
	///amount of used descriptors in the table
	u32 _length;
};

///A gate type 
enum class GateType: u8 {
	TaskGate32 = 5,
	InterruptGate16 = 6,
	TrapGate16 = 7,
	InterruptGate32 = 14,
	TrapGate32 = 15,
};

void IdtcreateEmpty();
void IdtFlush();
extern "C" void __attribute__ ((noinline)) IdtRegisterInterrupts();
extern "C" void reload_idt(u16 limit, u32 base);