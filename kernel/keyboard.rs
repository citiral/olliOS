///The amount of keys
const KEYS: usize = 255;

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

///this struct manages the low level aspects of the PS/2 keyboard
///it receives interrupt scancodes through a callback function, and, from there
///manages everything
pub struct Keyboard {
	keystates: [u8;KEYS]
}

impl Keyboard {

	pub fn new() -> Keyboard
	{
		Keyboard {
			keystates: [0;KEYS],
		}
	}

	pub fn register_callback(&self)
	{

	}
	
}


//ideas:
// callback of interrupts by lambda expressions (can capture environment so callbacks can work on structs)