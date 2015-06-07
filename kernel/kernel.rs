#![feature(no_std, core, lang_items)]
#![no_std]

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

fn assert_correctness()
{
	arch::vga::assert_correctness();
	arch::interrupt::assert_correctness();
}

#[no_mangle]
pub fn main()
{
	//when booting, clear the screen and show the splash
	unsafe {
		vga::global_writer.clear();
		vga_println!("Booting olliOS, greetings from Rust!");
		//next step, make sure the binary is fine 
		vga_println!("Asserting correctness.");
	}
	
	assert_correctness();

	//we reached the end of the main, so the kernel is ending. Shouldn't really happen but w/e
	unsafe {
		vga_println!("End of kernel reached.");
	}
}
