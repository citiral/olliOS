#[allow(unused_imports)]
use prelude::*;

#[macro_export]
macro_rules! vga_print {
    ($fmt:expr) => {
        (write!(::arch::vga::global_writer, $fmt)).unwrap();
    };

    ($fmt:expr, $($msg:tt)*) => {
        (write!(::arch::vga::global_writer, $fmt, $($msg)*)).unwrap();
    };
}

#[macro_export]
macro_rules! vga_println {
    ($fmt:expr) => (
        vga_print!(concat!($fmt, "\n"));
    );

    ($fmt:expr, $($msg:tt)*) => (
        vga_print!(concat!($fmt, "\n"), $($msg)*);
    );
}

#[macro_export]
macro_rules! assert_size {
    ($t:ty,$size:expr) => {
        assert!(::core::mem::size_of::<$t>() == $size, concat!(stringify!($t), " is {} bytes instead of {}"), ::core::mem::size_of::<$t>(), $size);
    }
}

#[macro_export]
macro_rules! label_addr {
    ($texp:ident) => {
        &mut $texp as *mut ::types::Label
    }
}

#[macro_export]
macro_rules! label_val {
    ($texp:ident) => {
        $texp
    }
}