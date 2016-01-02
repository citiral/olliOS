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
				let pressedkey = match byte {
					0x1C => Some(Vkey::A),
					0x32 => Some(Vkey::B),
					0x21 => Some(Vkey::C),
					0x23 => Some(Vkey::D),
					0x24 => Some(Vkey::E),
					0x2B => Some(Vkey::F),
					0x34 => Some(Vkey::G),
					0x33 => Some(Vkey::H),
					0x43 => Some(Vkey::I),
					0x3B => Some(Vkey::J),
					0x42 => Some(Vkey::K),
					0x4B => Some(Vkey::L),
					0x3A => Some(Vkey::M),
					0x31 => Some(Vkey::N),
					0x44 => Some(Vkey::O),
					0x4D => Some(Vkey::P),
					0x15 => Some(Vkey::Q),
					0x2D => Some(Vkey::R),
					0x1B => Some(Vkey::S),
					0x2C => Some(Vkey::T),
					0x3C => Some(Vkey::U),
					0x2A => Some(Vkey::V),
					0x1D => Some(Vkey::W),
					0x22 => Some(Vkey::X),
					0x35 => Some(Vkey::Y),
					0x1A => Some(Vkey::Z),
					_ => None,
				};

				if let Some(key) = pressedkey {
					unsafe {
						vga_println!("Key pressed");
					}
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
	}
}

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
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F9,
	F10,
	F11,
	F12,
}