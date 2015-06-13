use prelude::*;
use core::fmt::Write;
use core::mem::size_of;

#[repr(C, packed)]
#[derive(Debug, Copy, Clone)]
///Represents an IDT pointer that can be stord in the IDTR register
pub struct DescriptorTablePointer {
	///the length of the IDT in bytes - 1; Ie: 0x1000 means 0x200 interrupts
	pub limit: u16,
	///address of where the IDT is stored
	pub base: u32,
}

#[derive(Copy, Clone, Debug)]
///A struct representing the Descriptor Privilege Level.
///the highest level is 0 and the lowest level is 3
pub enum Dpl {
	Ring0 = 0,
	Ring1 = 1,
	Ring2 = 2,
	Ring3 = 3,
}

///the used asm functions for the descriptors
extern "C" {

	//initializes the task state register
	pub fn initialize_tss(datasegment: u16, gdtindex: u16);
}

///Calling this will check if all parts of this module are working correctly (proper sizes, ...)
///If this function fails it probably means the kernel has been improperly compiled and ignored some attributes, liked packed
pub fn assert_correctness()
{
	assert_size!(DescriptorTablePointer, 6);
}