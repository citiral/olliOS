#![feature(no_std)]
#![no_std]
#![crate_type="staticlib"]
#![feature(core)]
#![feature(asm)]
#![feature(lang_items)]


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
pub mod error;
pub mod libc;
pub mod types;
pub mod keyboard;


///The base kernel struct containing everything
struct Kernel {
	gdt: gdt::Gdt,
	keyboard: keyboard::Keyboard,
}

impl Kernel {
	pub fn initialize(&mut self)
	{
		unsafe {
			vga_println!("Setting up flat GDT.");
		}
		self.gdt = gdt::create_flat_gdt();
		unsafe {
			self.gdt.flush();
		}
	}
}

fn assert_correctness()
{
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
		vga_println!("Asserting correctness.");
	}
	assert_correctness();

	unsafe {
		vga_println!("Booting olliOS, greetings from Rust!");
	}

	let mut kernel = Kernel {
		gdt: gdt::Gdt::new(),
		keyboard: keyboard::Keyboard::new(),
	};
	kernel.initialize();

	unsafe {
		vga_println!("Registering interrupts.");
		interrupt::register_interrupts();
	}
	
	unsafe {
		vga_println!("Initialising GDT, TSS and IDT");
	}

	descriptor::init_flat_gdt();

	unsafe {
		vga_println!("Initialising the PIC");
		pic::init_pic();

		//TODO move this to a keyboard class (this disables translation)
		io::outb(0x64, 0x60);
		io::outb(0x60, 0b00000001);
		//io::inb(0x60);
	}

	//we reached the end of the main, so the kernel is ending. Shouldn't really happen but w/e
	unsafe {
		vga_println!("End of kernel reached.");
	}

	loop {
		unsafe {
			asm!("hlt");
		}
	}
}
