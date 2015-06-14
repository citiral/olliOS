#![feature(no_std, core, box_syntax, asm, lang_items)]
#![no_std]
#![crate_type="staticlib"]
#![allow(dead_code)]


///we use the core crate instead of the std one
#[macro_use]
extern crate core;

#[allow(unused_imports)]
use prelude::*;

pub use arch::*;
use core::fmt::Write;

#[macro_use] pub mod macros;
#[path="../arch/i686/rust/lib.rs"] pub mod arch;
pub mod prelude;
pub mod langitems;
pub mod libc;
pub mod types;
pub mod keyboard;
pub mod alloc;


///The base kernel struct containing everything
struct Kernel {
	keyboard: keyboard::Keyboard,
}

impl Kernel {
}

///initializes the hardware abstraction layer
fn initialize_hal()
{
	unsafe {
		vga_println!("beginning initializing HAL.");
		vga_println!("Initializing GDT.");

		gdt::GDT = gdt::create_flat_gdt();
		gdt::GDT.flush();
		
		vga_println!("Initializing IDT.");
		interrupt::IDT = interrupt::create_empty_idt();
		interrupt::IDT.flush();
		interrupt::register_interrupts();

		vga_println!("Initializing TSS.");
		descriptor::initialize_tss(0x10, 0x28);
	}
}

fn assert_correctness()
{
	unsafe {
		vga_println!("Asserting correctness.");
	}
	vga::assert_correctness();
	descriptor::assert_correctness();
	interrupt::assert_correctness();
	gdt::assert_correctness();
}

#[no_mangle]
pub fn main()
{
	//when booting, clear the screen and show the splash
	unsafe {
		vga::global_writer.clear();
	}

	assert_correctness();
	initialize_hal();
	
	unsafe {
		vga_println!("Initialising the PIC");
		pic::init_pic();

		//TODO move this to a keyboard class (this disables translation from keyset 2 to 1)
		io::outb(0x64, 0x60);
		io::outb(0x60, 0b00000001);
		//io::inb(0x60);
	}

	loop {
		unsafe {
			vga_println!("We are halting!");
			asm!("hlt");
		}
	}
}
