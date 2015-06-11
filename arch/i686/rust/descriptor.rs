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

///reloads the gdt register with the given gdt length
fn flush_gdt(length: usize)
{
	unsafe {
		/*let pointer = DescriptorTablePointer {
			limit: (length * size_of::<DescriptorTablePointer>()) as u16,
			base: (&GDT).as_ptr() as u32,
		};*/
		
		//asm!("lgdt %eax"::"{eax}"(pointer));

		reload_tss();
		//reload_idt();
		
		//vga_println!("beep");
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
	pub fn reload_tss();
}

///sets up a flat GDT. Everything is mapped from 0 to 0xFFFFFFFF
pub fn init_flat_gdt()
{
	unsafe {
		//set the zero gdt
		/*GDT[0].set_base(0);
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
*/
		flush_gdt(3);
	}
}

///Calling this will check if all parts of this module are working correctly (proper sizes, ...)
///If this function fails it probably means the kernel has been improperly compiled and ignored some attributes, liked packed
pub fn assert_correctness()
{
	assert_size!(DescriptorTablePointer, 6);
}