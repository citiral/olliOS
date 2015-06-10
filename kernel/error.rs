use core::fmt::{Write, Arguments};


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

#[lang = "panic_fmt"]
fn panic_impl(fmt: Arguments, file: &'static str, line: u32) -> !
{
	unsafe {
		vga_println!("[PANIC] {} at line {}: {}", file, line, fmt);
	}

	loop {}
}
