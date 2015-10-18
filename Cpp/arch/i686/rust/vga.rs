use prelude::*;
use core::{self, fmt};
use io::*;

///The pointer to the vga buffer
const TEXT_POINTER: *mut u16 = 0xB8000 as *mut u16;
const WIDTH: usize = 80;
const HEIGHT: usize = 25;

///A static vga text writer that can be used to write text from anyway. Using this is very unsafe, though
pub static mut global_writer: VgaTextWriter = VgaTextWriter {
	column: 0,
	row: 0, 
	foreground_color: Color::White,
	background_color: Color::Black,
	blinking: false,
};

#[derive(Copy, Clone)]
#[repr(u8)]
///Enum wrapping all possible text mode vga colors
pub enum Color {
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Magenta = 5,
	Brown = 6,
	LightGrey = 7,
	DarkGrey = 8,
	LightBlue = 9,
	LightGreen = 10,
	LightCyan = 11,
	LightRed = 12,
	LightMagenta = 13,
	LightBrown = 14,
	White = 15,
}

///A writer that is capable of writing to the VGA text buffer located at TEXT_POINTER
///Beware that this writer is not safe since multiple writers might be interacting with vga
///simultaniously
pub struct VgaTextWriter {
	column: usize,
	row: usize,
	pub foreground_color: Color,
	pub background_color: Color,
	blinking: bool,

}

impl VgaTextWriter {
	///creates a new vgatextwriter
	///it has default values of column & row 0
	///the foreground color is white and background color is black
	///by default there is no blinking
	pub fn new() -> VgaTextWriter
	{
		VgaTextWriter {
			column: 0,
			row: 0, 
			foreground_color: Color::White,
			background_color: Color::Black,
			blinking: false,
		}
	}

	///creates a vga entry for the writer
	fn generate_entry(&self, value: char) -> u16
	{
		let ce = value as u16;
		let fe = (self.foreground_color as u16) << 8;
		let be = (self.background_color as u16) << 12;
		let ble = (self.blinking as u16) << 15;
		ce | fe | be | ble
	}

	///scrolls the screen one line up and clears the bottom line
	pub fn scroll_up(&mut self)
	{
		//scroll the screen
		for row in 0..HEIGHT - 1 {
			for column in 0..WIDTH {
				unsafe{
					let newvalue = *TEXT_POINTER.offset(((row+1)*WIDTH + column) as isize);
					*TEXT_POINTER.offset((row*WIDTH + column) as isize) = newvalue;
				}
			}
		}
		
		//and clear the bottom row
		for column in 0..WIDTH {
			unsafe{
				self.put_char(' ', column, HEIGHT - 1);
			}
		}
	}

	///scrolls the screen one line up and clears the bottom line
	pub fn scroll_down(&mut self)
	{
		//scroll the screen
		let mut row = HEIGHT-1;

		while row > 0 {
			for column in 0..WIDTH {
				unsafe{
					let newvalue = *TEXT_POINTER.offset(((row-1)*WIDTH + column) as isize);
					*TEXT_POINTER.offset((row*WIDTH + column) as isize) = newvalue;
				}
			}
			row -= 1;
		}
		
		//and clear the bottom row
		for column in 0..WIDTH {
			unsafe{
				self.put_char(' ', column, 0);
			}
		}
	}

	pub unsafe fn put_char(&self, value: char, column: usize, row: usize)
	{
		*TEXT_POINTER.offset((row * WIDTH + column) as isize) = self.generate_entry(value);
	}

	///writes a single character to the text writer
	pub unsafe fn write_char(&mut self, value: char)
	{
		match value {
			'\n' => {
				self.row += 1;
				self.column = 0;

				if self.row == HEIGHT {
					self.scroll_up();
					self.row -= 1;
				}
				let (c, r) = (self.column, self.row);
				self.set_cursor(c as u8, r as u8);
			}
			_ => {
				self.put_char(value, self.column, self.row);
				self.column += 1;
				let (c, r) = (self.column, self.row);
				self.set_cursor(c as u8, r as u8);
			}		
		}
		//wrap if we are now at the end of the line
		if self.column == WIDTH {
			self.row += 1;
			self.column = 0;
		}
	}

	pub unsafe fn write_string(&mut self, string: &str)
	{
		for c in string.chars() {
			self.write_char(c);
		}
	}

	///clears the vga buffer and resets the position to 0, 0
	pub unsafe fn clear(&mut self)
	{
		self.column = 0;
		self.row = 0;

		for x in 0..WIDTH {
			for y in 0..HEIGHT {
				self.put_char(' ', x, y);
			}
		}
	}

	pub fn set_cursor(&mut self, x: u8, y: u8)
	{
		let position = (x as u16) + (y as u16) * 80;
		unsafe {
			//set position low byte
			outb(0x3D4, 0x0F);
			outb(0x3D5, position as u8);

			//and position high byte
			outb(0x3D4, 0x0E);
			outb(0x3D5, (position >> 8) as u8);
		}
	}
}

impl fmt::Write for VgaTextWriter {
    fn write_str(&mut self, s: &str) -> fmt::Result
    {
    	unsafe {
    		self.write_string(s);
    		Ok(())
    	}
    }
}

pub fn assert_correctness()
{
	assert!(core::mem::size_of::<Color>() == 1, "The Color struct is {} bytes instead of 1.", core::mem::size_of::<Color>());
}