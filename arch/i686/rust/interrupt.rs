#[allow(unused_imports)]
use prelude::*;
use descriptor;
use core::fmt::Write;
use pic;

const IDT_ENTRIES: usize = 255;
static IDT: [IdtDescriptor;IDT_ENTRIES] = [IdtDescriptor {offset_low: 0, selector: 0, zero: 0, type_attr: 0, offset_hi: 0};IDT_ENTRIES];

#[derive(Copy, Clone, Debug)]
#[repr(C, packed)]
///A single entry in the Idt table
struct IdtDescriptor {
	offset_low: u16,
	selector: u16,
	zero: u8,
	type_attr: u8,
	offset_hi: u16,
}

#[derive(Copy, Clone, Debug)]
#[repr(u8)]
///A gate type 
enum GateType {
	TaskGate32 = 5,
	InterruptGate16 = 6,
	TrapGate16 = 7,
	InterruptGate32 = 14,
	TrapGate32 = 15,
}

impl IdtDescriptor {
	fn new() -> IdtDescriptor
	{
		IdtDescriptor {
			offset_low: 0,
			selector: 0,
			zero: 0,
			type_attr: 0,
			offset_hi: 0,
		}
	}

	fn set_offset(&mut self, offset: u32)
	{
		self.offset_hi = (offset >> 16) as u16;
		self.offset_low = offset as u16;
	}

	fn set_paging(&mut self, enabled: bool)
	{
		if enabled {
			self.type_attr |= 0b10000000;
		} else {
			self.type_attr &= 0b01111111;
		}
	}

	fn set_dpl(&mut self, dpl: descriptor::Dpl)
	{
		let value = dpl as u8;
		self.type_attr = (self.type_attr & 0b10011111) | value << 5;
	}

	fn set_storage_segment(&mut self, enabled: bool)
	{
		if enabled {
			self.type_attr |= 0b00010000;
		} else {
			self.type_attr &= 0b11101111;
		}
	}

	fn set_gatetype(&mut self, gatetype: GateType)
	{
		let value = gatetype as u8;
		self.type_attr = (self.type_attr & 0b11110000) | value;
	}

	fn set_selector(&mut self, selector: u16)
	{
		self.selector = selector;
	}
}

#[no_mangle]
pub fn rust_int_unused()
{
	unsafe {
		vga_println!("I got an unknown interrupt");
		for x in 0..17 {
			pic::end_interrupt(x as u8);
		}
	}
}

///Calling this will check if all parts of this module are working correctly (proper sizes, ...)
///If this function fails it probably means the kernel has been improperly compiled and ignored some attributes, liked packed
pub fn assert_correctness()
{
	assert_size!(IdtDescriptor, 8);
}