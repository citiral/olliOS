use prelude::*;
use io;
use pic;
use macros;
use core::fmt::Write;
use interrupt;

const PS2_INOUT: u16 = 0x60;
const PS2_RW: u16 = 0x64;
const KEYB_INT: usize = 0x21;

///The amount of keys
const KEYS: usize = 255;

pub static mut KEYBOARD: Keyboard = Keyboard {
	keystates: [0;KEYS],
	currentState: InputState::WaitingForNewCommand,
};

pub enum Vkey {
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Number0,
	Number1,
	Number2,
	Number3,
	Number4,
	Number5,
	Number6,
	Number7,
	Number8,
	Number9,
	Numpad0,
	Numpad1,
	Numpad2,
	Numpad3,
	Numpad4,
	Numpad5,
	Numpad6,
	Numpad7,
	Numpad8,
	Numpad9,
	Backspace,
	Enter,
	Tab,
	LShift,
	RShift,
	LControl,
	RControl,
	LAlt,
	RAlt,
	Mod,
	CapsLock,
	PageUp,
	PageDown,
	Home,
	End,
	ArrowLeft,
	ArrowRight,
	ArrowUp,
	ArrowDown,
	Insert,
	Delete,
	Space,
}

enum InputState {
	WaitingForNewCommand,
	WaitingForReleasedKey,
}

///this struct manages the low level aspects of the PS/2 keyboard
///it receives interrupt scancodes through a callback function, and, from there
///manages everything
pub struct Keyboard {
	keystates: [u8;KEYS],
	currentState: InputState,
}

impl Keyboard {

	///actives the interrupts on this keyboard
	pub fn make_active(&'static mut self)
	{
		unsafe {
			interrupt::IDT.set_function(KEYB_INT, self);
		}
	}

	fn handle_new_commnand(&mut self, byte: u8)
	{
		match byte {
			//this means we are getting a released button command. oh boi!
			0xF0 => self.currentState = InputState::WaitingForReleasedKey,
			//anything else is probably a keyinput
			_ => {
				let key = match byte {
					0x1C => 'A',
					0x32 => 'B',
					0x21 => 'C',
					0x23 => 'D',
					0x24 => 'E',
					0x2B => 'F',
					0x34 => 'G',
					0x33 => 'H',
					0x43 => 'I',
					0x3B => 'J',
					0x42 => 'K',
					0x4B => 'L',
					0x3A => 'M',
					0x31 => 'N',
					0x44 => 'O',
					0x4D => 'P',
					0x15 => 'Q',
					0x2D => 'R',
					0x1B => 'S',
					0x2C => 'T',
					0x3C => 'U',
					0x2A => 'V',
					0x1D => 'W',
					0x22 => 'X',
					0x35 => 'Y',
					0x1A => 'Z',
					_ => '\0'
				};
				if key != '\0' {
					unsafe { ::vga::global_writer.write_char(key) };
				}
			}
		}
	}

	fn handle_waiting_for_released_key(&mut self, byte: u8)
	{
		self.currentState = InputState::WaitingForNewCommand;
	}

	fn handle_byte(&mut self, byte: u8)
	{
		match self.currentState {
			InputState::WaitingForNewCommand => {
				self.handle_new_commnand(byte);
			},
			InputState::WaitingForReleasedKey => {
				self.handle_waiting_for_released_key(byte);
			}
		}
	}
}

impl FnOnce<()> for Keyboard {
	type Output = ();

	extern "rust-call" fn call_once(self, args: ()) -> Self::Output {
		()
	}
}

impl FnMut<()> for Keyboard {
	extern "rust-call" fn call_mut(&mut self, args: ()) -> ()
	{
		unsafe {
			let scan = io::inb(PS2_INOUT);
			self.handle_byte(scan);
			pic::end_interrupt(1);
		}

		/*unsafe {
			let scan = io::inb(PS2_INOUT);
			handle_byte(scan);
			if (scan == 0xF0) {
				let key = io::inb(PS2_INOUT);
			} else {

				let key = match scan {
					0x1C => 'A',
					0x32 => 'B',
					0x21 => 'C',
					0x23 => 'D',
					0x24 => 'E',
					0x2B => 'F',
					0x34 => 'G',
					0x33 => 'H',
					0x43 => 'I',
					0x3B => 'J',
					0x42 => 'K',
					0x4B => 'L',
					0x3A => 'M',
					0x31 => 'N',
					0x44 => 'O',
					0x4D => 'P',
					0x15 => 'Q',
					0x2D => 'R',
					0x1B => 'S',
					0x2C => 'T',
					0x3C => 'U',
					0x2A => 'V',
					0x1D => 'W',
					0x22 => 'X',
					0x35 => 'Y',
					0x1A => 'Z',
					_ => '\0'
				};
				if key != '\0' {
					::vga::global_writer.write_char(key);
				}
			}

			//vga_println!("Keyboard interrupt scan {:x}", scan);
			//io::outb(0x20, 0x20);
			pic::end_interrupt(1);
			
			}
		()*/
	}
}

//ideas:
// callback of interrupts by lambda expressions (can capture environment so callbacks can work on structs)