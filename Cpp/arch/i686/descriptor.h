#pragma once
#include "types.h"


///A struct representing the Descriptor Privilege Level.
///the highest level is 0 and the lowest level is 3
enum class Dpl: u8 {
	Ring0 = 0,
	Ring1 = 1,
	Ring2 = 2,
	Ring3 = 3,
};

///Represents an IDT pointer that can be stord in the IDTR register
struct DescriptorTablePointer {
	///the length of the IDT in bytes - 1; Ie: 0x1000 means 0x200 interrupts
	u16 limit;
	///address of where the IDT is stored
	u32 base;
};

//the used asm functions for the descriptors
//initializes the task state register
extern "C" void initialize_tss(u16 datasegment, u16 gdtindex);