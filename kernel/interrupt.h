#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "descriptor.h"
#include "types.h"
#include "kstd/vector.h"
#include "kstd/utility.h"

#define MAX_IDT_ENTRIES 256
#define THROW_INTERRUPT(x) __asm__ volatile("int $" #x);
#define INT_KEYBOARD 0x21
#define INT_ATA_BUS1 46
#define INT_ATA_BUS2 47
#define INT_GENERAL_PROTECTION_VIOLATION 0x0D
#define INT_PAGE_FAULT 0x0E
#define INT_TIMER 32
#define INT_PREEMPT 33
#define INT_SPURIOUS 0xFF


typedef void (*InterruptCallback)(u32);

// An IRQ function receives a u32 containing the number of the interrupt that fired,
// a pointer to the interrupt stack so that extra data could be read if necessary,
// a pointer to a possible extra object (e.g. a Device object. Can also be null if not used),
// and will return one value.
// Because more than one device can share an IRQ a handler should return false
// if the interrupt is not caused by the device the handler is handling.
// The interrupt driver will the ask other handlers if they can handle.
// However of the handler has handled the IRQ it should return true so the interrupt
// driver knows it can stop.
typedef bool (*IRQ)(u32, void*, void*);

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

class Interrupts {
public:
	void registerIRQ(u32 irq, IRQ handler, void* obj);
	void unregisterIRQ(u32 irq, IRQ handler, void* obj);
	void callIRQ(u32 irq, void* stack);

private:
	std::vector<IRQ> _callbacks[MAX_IDT_ENTRIES];
	std::vector<void*> _objects[MAX_IDT_ENTRIES];
};

void interrupts_callRawIRQ(u32 irq);

void IdtcreateEmpty();
void IdtFlush();
u32 IdtBase();
u16 IdtLimit();
extern Interrupts interrupts;
extern Idt idt;

extern "C" void __attribute__ ((noinline)) IdtRegisterInterrupts();
extern "C" void reload_idt(u16 limit, u32 base);
extern "C" void end_interrupt(u32 irq);

#endif /* end of include guard: __INTERRUPT_H */
