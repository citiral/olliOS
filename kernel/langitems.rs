use core::fmt::{Write, Arguments};
use core::raw::{self, Repr};
use alloc::{self, Allocator};

#[lang="stack_exhausted"]
extern fn stack_exhausted() {
    loop {}
}


#[lang="eh_personality"]
extern fn eh_personality() {
    loop {}
} 

#[cold] #[inline(never)]
///function is ripped from libcore (panicking.rs)
///For some god forsaken reason this function needs to be redefined here or else panic_impl doesn't really work.
///To anyone reading this code, it's probably something very stupid i'm doing that is causing this and this is probably only half a fix, if even that.
///If you know what the real problem is, please let me know!
pub fn panic_fmt(fmt: Arguments, file_line: &(&'static str, u32)) -> ! {
    #[allow(improper_ctypes)]
    extern {
        #[lang = "panic_fmt"]
        fn panic_impl(fmt: Arguments, file: &'static str, line: u32) -> !;
    }
    let (file, line) = *file_line;
    unsafe { panic_impl(fmt, file, line) }
}


#[lang = "panic_fmt"]
fn panic_impl(fmt: Arguments, file: &'static str, line: u32) -> !
{
	unsafe {
		vga_println!("[PANIC] {} at line {}: {}", file, line, fmt);
	}

	loop {}
}

#[lang = "owned_box"]
pub struct Box<T>(*mut T);

static mut ALLOC: alloc::WaterMarkAllocator = alloc::WaterMarkAllocator {
	heap: [0; alloc::REC_HEAP_SIZE],
	counter: 0,
};

#[lang = "exchange_malloc"]
unsafe fn allocate(size: usize, _align: usize) -> *mut u8 {
	ALLOC.allocate(size, _align)
}
#[lang = "exchange_free"]
unsafe fn deallocate(ptr: *mut u8, _size: usize, _align: usize) {
    ALLOC.deallocate(ptr, _size, _align)
}
