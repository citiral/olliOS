use prelude::*;
use core::fmt::Write;
use core::mem::size_of;
const GDT_ENTRIES: usize = 255;
static mut GDT: [GdtDescriptor;GDT_ENTRIES] = [GdtDescriptor {
	limit: 0,
	base_low: 0,
	base_mid: 0,
	access: 16,
	limit_flags: 0,
	base_hi: 0,
};GDT_ENTRIES];


#[repr(C, packed)]
#[derive(Debug, Copy, Clone)]
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
	access: u8,
	limit_flags: u8,
	base_hi: u8,
}

impl GdtDescriptor {
	fn new() -> GdtDescriptor
	{
		GdtDescriptor {
			limit: 0,
			base_low: 0,
			base_mid: 0,
			access: 16,
			limit_flags: 0,
			base_hi: 0,
		}
	}

	///sets the limit of the Descriptor. The value is an u32, but only the lower 20 bits are used
	fn set_limit(&mut self, value: u32)
	{
		self.limit = value as u16;
		self.limit_flags = (self.limit_flags & 0b11110000) | (((value >> 16) as u8) & 0b00001111);
	}

	fn set_base(&mut self, value: u32)
	{
		self.base_low = value as u16;
		self.base_mid = (value >> 16) as u8;
		self.base_hi = (value >> 24) as u8;
	}

	fn set_present(&mut self, present: bool)
	{
		self.access = (self.access & 0b01111111) | ((present as u8) << 7); 
	}

	fn set_dpl(&mut self, dpl: Dpl)
	{
		let value = dpl as u8;
		self.access = (self.access & 0b10011111) | value << 5;
	}

	///sets the exectuable bit true = 1 = is executable, false = 0 = data
	fn set_executable(&mut self, exectuable: bool)
	{
		self.access = (self.access & 0b11110111) | ((exectuable as u8) << 3); 
	}

	///sets the direction bit
	fn set_direction(&mut self, up: bool)
	{
		self.access = (self.access & 0b11111011) | ((up as u8) << 2); 
	}

	fn set_rw(&mut self, writeable: bool)
	{
		self.access = (self.access & 0b11111101) | ((writeable as u8) << 1); 
	}

	///if the page is true the granularity is set to page granularity (4KB) otherwise it is in bytes
	fn set_granularity(&mut self, page: bool)
	{
		self.limit_flags = (self.limit_flags & 0b01111111) | ((page as u8) << 7);
	}

	///if 0 this is a 16 bit selector, else it is a 32 bit selector
	fn set_size(&mut self, size: bool)
	{
		self.limit_flags = (self.limit_flags & 0b10111111) | ((size as u8) << 6);
	}

	//directly set the access flags
	fn set_type(&mut self, access: u8)
	{
		self.access = access;
	}
}

///reloads the gdt register with the given gdt length
fn flush_gdt(length: usize)
{
	unsafe {
		let pointer = DescriptorTablePointer {
			limit: (length * size_of::<DescriptorTablePointer>()) as u16,
			base: (&GDT).as_ptr() as u32,
		};
		
		//asm!("lgdt %eax"::"{eax}"(pointer));
		vga_println!("beep");

		reload_segments();
		reload_tss();
		reload_idt();

/*
		let raw : u64 = pointer.limit as u64 + ((pointer.base as u32 as u64) << 16);

		let newpointer = DescriptorTablePointer {
			limit: (raw) as u16,
			base:  ((gdt_pointer >> 16) as u32),
		};

		vga_println!("old pointer is {:?} new pointer is {:?} ", raw as u32, gdt_pointer as u32);
*/
	}
}

///the used asm functions for the descriptors
extern "C" {
	pub fn reload_segments();
	pub fn reload_tss();
	pub fn reload_idt();
}

///sets up a flat GDT. Everything is mapped from 0 to 0xFFFFFFFF
pub fn init_flat_gdt()
{
	unsafe {
		//set the zero gdt
		GDT[0].set_base(0);
		GDT[0].set_limit(0);
		GDT[0].access = 16;
		GDT[0].limit_flags = 0;

		//set the code GDT
		GDT[1].set_base(0);
		GDT[1].set_limit(0xFFFFFFFF);
		GDT[1].set_present(true);
		GDT[1].set_dpl(Dpl::Ring0);
		GDT[1].set_executable(true);
		GDT[1].set_direction(false);
		GDT[1].set_rw(true);
		GDT[1].set_granularity(true);
		GDT[1].set_size(true);

		//set the code GDT
		GDT[2].set_base(0);
		GDT[2].set_limit(0xFFFFFFFF);
		GDT[2].set_present(true);
		GDT[2].set_dpl(Dpl::Ring0);
		GDT[2].set_executable(false);
		GDT[2].set_direction(false);
		GDT[2].set_rw(true);
		GDT[2].set_granularity(true);
		GDT[2].set_size(true);

		assert!(GDT[1].limit == 0xFFFF, "limit wrong size");
		assert!(GDT[1].access == 0x9A, "Access flags are wrong: should be {:b} but are {:b}.", 0x9A, GDT[1].access);
		assert!(GDT[2].access == 0x92, "Access flags are wrong: should be {:b} but are {:b}.", 0x92, GDT[2].access);
		assert!((GDT[1].limit_flags) == 0b11001111, "limit flags are wrong: should be {:b} but are {:b}.", 0b11001111, GDT[1].limit_flags);

		flush_gdt(3);
	}
}

///Calling this will check if all parts of this module are working correctly (proper sizes, ...)
///If this function fails it probably means the kernel has been improperly compiled and ignored some attributes, liked packed
pub fn assert_correctness()
{
	assert_size!(DescriptorTablePointer, 6);
	assert_size!(GdtDescriptor, 8);
}