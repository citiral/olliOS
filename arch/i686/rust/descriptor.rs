use prelude::*;

const GDT_ENTRIES: usize = 255;
static GDT: [GdtDescriptor;GDT_ENTRIES] = [GdtDescriptor {
	limit: 0,
	base_low: 0,
	base_mid: 0,
	acess: 0,
	limit_flags: 0,
	base: 0,
};GDT_ENTRIES];


#[repr(C, packed)]
///Represents an IDT pointer that can be stord in the IDTR register
pub struct DescriptorTablePointer {
	///the length of the IDT in bytes - 1; Ie: 0x1000 means 0x200 interrupts
	limit: u16,
	///address of where the IDT is stored
	base: u32,
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

#[derive(Copy, Clone, Debug)]
#[repr(C, packed)]
struct GdtDescriptor {
	limit: u16,
	base_low: u16,
	base_mid: u8,
	acess: u8,
	limit_flags: u8,
	base: u8,
}

impl GdtDescriptor {
	
}

pub fn init_flat_gdt() {

}

///Calling this will check if all parts of this module are working correctly (proper sizes, ...)
///If this function fails it probably means the kernel has been improperly compiled and ignored some attributes, liked packed
pub fn assert_correctness()
{
	assert_size!(DescriptorTablePointer, 6);
}