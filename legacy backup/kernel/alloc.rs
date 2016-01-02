use core::raw::{self, Repr};
use macros;
use core::fmt::Write;
use core::ops;
///this file contains everything memory allocation related, it contains one or more different kinds of memory allocators that can be used 
///inside the kernel

///the size of the recommended heap of an allocator
///not neccessarily the actual size of the heap
pub const REC_HEAP_SIZE: usize = 1024*1024; //1 megabyte

///Trait describing a generic memory allocator
pub trait Allocator {
	unsafe fn allocate(&mut self, size: usize, align: usize) -> *mut u8;
	unsafe fn deallocate(&mut self, ptr: *mut u8, size: usize, align: usize);
}

pub struct Box<T> {
	value: T
}

impl<T> ops::Deref for Box<T> {
	type Target = T;

	fn deref<'a>(&'a self) -> &'a T {
        &self.value
    }
}

///A watermark allocator. The simplest of allocators, it never frees memory, it only keeps counting up and up.
///Useful for debugging purposes, since you can be almost guaranteed this works
pub struct WaterMarkAllocator {
	pub heap: [u8; REC_HEAP_SIZE],
	pub counter: usize,
}

impl WaterMarkAllocator {
	fn new() -> WaterMarkAllocator
	{
		WaterMarkAllocator {
			heap: [0; REC_HEAP_SIZE],
			counter: 0,
		}
	}
}

impl Allocator for WaterMarkAllocator {
	unsafe fn allocate(&mut self, size: usize, align: usize) -> *mut u8
	{
		vga_println!("allocating");
		let repr = self.heap.repr();
		let base = (repr.data as usize) + self.counter;
		//get offset from being alligned
		let offset = align - (base % align);
		//get the new address
		let newaddr = base + offset + size;
		//move the counter
		self.counter += offset + size;
		//check that we are not out of memory.
		assert!(newaddr < (repr.data as usize) + repr.len, "Out of heap memory: size: {}, align: {}", size, align);
		newaddr as *mut u8
	}

	unsafe fn deallocate(&mut self, ptr: *mut u8, size: usize, align: usize)
	{
		vga_println!("deallocating");
		//deallocation? never heard of it.
	}
}