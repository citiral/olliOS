#[allow(unused_imports)]
use prelude::*;
use descriptor;
use core::fmt::Write;
use pic;
use types::Label;
use io;
use core::raw::{self, Repr};
use core::mem;
use core::ptr;

const MAX_IDT_ENTRIES: usize = 256;

pub static mut IDT: Idt<'static> = Idt {
	table: [IdtDescriptor {
		offset_low: 0,
		selector: 0,
		zero: 0,
		type_attr: 0,
		offset_hi: 0,
	};MAX_IDT_ENTRIES],
	callbacks: [None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None],
	length: 0
};

pub struct Idt<'a> {
	///The table containing the gdt descriptors
	table: [IdtDescriptor;MAX_IDT_ENTRIES],
	///the list of callbacks linked to the interrupts
	callbacks: [Option<&'a mut FnMut()>; MAX_IDT_ENTRIES],
	///amount of used descriptors in the table
	length: usize,
}

impl<'a> Idt<'a> {
		///generates a new gdt filled with empty entries
	pub fn new() -> Idt<'a> {
		Idt {
			table: [IdtDescriptor {
				offset_low: 0,
				selector: 0,
				zero: 0,
				type_attr: 0,
				offset_hi: 0,
			};MAX_IDT_ENTRIES],
			callbacks: [None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None],
			length: 0
		}
	}

	pub fn set_function(&mut self, index: usize, func: &'a mut FnMut())
	{
		self.callbacks[index] = Some(func);
	}

	pub fn call_function(&mut self, index: usize)
	{
		//call the function if it exists
		match self.callbacks[index] {
			Some(ref mut f) => f(),
			None => ()
		}
		//this also works: 
		//self.callbacks[index].as_mut().map(|f| f());
	}

	pub fn get_entry(&mut self, index: usize) -> &mut IdtDescriptor
	{
		&mut self.table[index]
	}

	pub fn set_entry(&mut self, index: usize, entry: IdtDescriptor)
	{
		self.table[index] = entry;
	}

	pub fn add_entry(&mut self, entry: IdtDescriptor)
	{
		self.table[self.length] = entry;
		self.length += 1;
	}

	pub fn set_length(&mut self, length: usize)
	{
		self.length = length
	}

	pub fn get_length(&mut self) -> usize
	{
		self.length
	}

	///flushes this IDT. It loads the idt pointer on the cpu
	pub unsafe fn flush(&self)
	{
		let pointer = self.generate_table_pointer();
		reload_idt(pointer.limit, pointer.base);
	}

	///generates a table descriptor pointer for this gdt
	fn generate_table_pointer(&self) -> descriptor::DescriptorTablePointer
	{
		let tableptr: raw::Slice<IdtDescriptor> = self.table.repr();
		descriptor::DescriptorTablePointer {
			base: tableptr.data as u32,
			limit: (self.length*8 - 1) as u16,
		}
	}
}

#[derive(Copy, Clone, Debug)]
#[repr(C, packed)]
///A single entry in the Idt table
pub struct IdtDescriptor {
	offset_low: u16,
	selector: u16,
	zero: u8,
	type_attr: u8,
	offset_hi: u16,
}

#[derive(Copy, Clone, Debug)]
#[repr(u8)]
///A gate type 
enum GateType {
	TaskGate32 = 5,
	InterruptGate16 = 6,
	TrapGate16 = 7,
	InterruptGate32 = 14,
	TrapGate32 = 15,
}

impl IdtDescriptor {
	fn new() -> IdtDescriptor
	{
		IdtDescriptor {
			offset_low: 0,
			selector: 0,
			zero: 0,
			type_attr: 0,
			offset_hi: 0,
		}
	}

	fn from_value(lower: u32, higher: u32) -> IdtDescriptor
	{
		IdtDescriptor {
			offset_low: lower as u16,
			selector: (lower >> 16) as u16,
			zero: higher as u8,
			type_attr: (higher >> 8) as u8,
			offset_hi: (higher >> 16) as u16,
		}
	}

	fn set_offset(&mut self, offset: u32)
	{
		self.offset_hi = (offset >> 16) as u16;
		self.offset_low = offset as u16;
	}

	fn set_paging(&mut self, enabled: bool)
	{
		if enabled {
			self.type_attr |= 0b10000000;
		} else {
			self.type_attr &= 0b01111111;
		}
	}

	fn set_dpl(&mut self, dpl: descriptor::Dpl)
	{
		let value = dpl as u8;
		self.type_attr = (self.type_attr & 0b10011111) | value << 5;
	}

	fn set_storage_segment(&mut self, enabled: bool)
	{
		if enabled {
			self.type_attr |= 0b00010000;
		} else {
			self.type_attr &= 0b11101111;
		}
	}

	fn set_gatetype(&mut self, gatetype: GateType)
	{
		let value = gatetype as u8;
		self.type_attr = (self.type_attr & 0b11110000) | value;
	}

	fn set_selector(&mut self, selector: u16)
	{
		self.selector = selector;
	}
}

pub fn create_empty_idt<'a>() -> Idt<'a>
{
	let mut newidt = Idt::new();
	for _ in 0..256 {
		let newint = IdtDescriptor::from_value(0x00080000, 0x00008E00);
		newidt.add_entry(newint);
	};
	newidt
}

macro_rules! generate_int_handler {
	($x: expr) => {{
		let address: u32;
		asm!(concat!("jmp 2f
			.align 4
		1:
			pushal
			cld
			pushl $$", stringify!($x),"
			call rust_int
			add $$4, %esp
			popal
			iret
		2:
			leal 1b, %eax"):"={eax}" (address):::"volatile","alignstack");
		IDT.get_entry($x).set_offset(address);
	}}
}

#[no_mangle]
pub extern "C" fn rust_int(interrupt: u32)
{
	unsafe {
		IDT.call_function(interrupt as usize);
	}
}

///the used asm functions
extern "C" {
	fn reload_idt(limit: u16, base: u32);
}


///Calling this will check if all parts of this module are working correctly (proper sizes, ...)
///If this function fails it probably means the kernel has been improperly compiled and ignored some attributes, liked packed
pub fn assert_correctness()
{
	assert_size!(Label, 4);
	assert_size!(*mut Label, 4);
	assert_size!(*mut u32, 4);
	assert_size!(u32, 4);
	assert_size!(IdtDescriptor, 8);
}

#[no_mangle]
#[inline(never)]
///registers all interrupts to the IDT
pub unsafe extern "C"  fn register_interrupts()
{
	unsafe {
		//first, set all addresses to unused
		generate_int_handler!(0);
		generate_int_handler!(1);
		generate_int_handler!(2);
		generate_int_handler!(3);
		generate_int_handler!(4);
		generate_int_handler!(5);
		generate_int_handler!(6);
		generate_int_handler!(7);
		generate_int_handler!(8);
		generate_int_handler!(9);
		generate_int_handler!(10);
		generate_int_handler!(11);
		generate_int_handler!(12);
		generate_int_handler!(13);
		generate_int_handler!(14);
		generate_int_handler!(15);
		generate_int_handler!(16);
		generate_int_handler!(17);
		generate_int_handler!(18);
		generate_int_handler!(19);
		generate_int_handler!(20);
		generate_int_handler!(21);
		generate_int_handler!(22);
		generate_int_handler!(23);
		generate_int_handler!(24);
		generate_int_handler!(25);
		generate_int_handler!(26);
		generate_int_handler!(27);
		generate_int_handler!(28);
		generate_int_handler!(29);
		generate_int_handler!(30);
		generate_int_handler!(31);
		generate_int_handler!(32);
		generate_int_handler!(33);
		generate_int_handler!(34);
		generate_int_handler!(35);
		generate_int_handler!(36);
		generate_int_handler!(37);
		generate_int_handler!(38);
		generate_int_handler!(39);
		generate_int_handler!(40);
		generate_int_handler!(41);
		generate_int_handler!(42);
		generate_int_handler!(43);
		generate_int_handler!(44);
		generate_int_handler!(45);
		generate_int_handler!(46);
		generate_int_handler!(47);
		generate_int_handler!(48);
		generate_int_handler!(49);
		generate_int_handler!(50);
		generate_int_handler!(51);
		generate_int_handler!(52);
		generate_int_handler!(53);
		generate_int_handler!(54);
		generate_int_handler!(55);
		generate_int_handler!(56);
		generate_int_handler!(57);
		generate_int_handler!(58);
		generate_int_handler!(59);
		generate_int_handler!(60);
		generate_int_handler!(61);
		generate_int_handler!(62);
		generate_int_handler!(63);
		generate_int_handler!(64);
		generate_int_handler!(65);
		generate_int_handler!(66);
		generate_int_handler!(67);
		generate_int_handler!(68);
		generate_int_handler!(69);
		generate_int_handler!(70);
		generate_int_handler!(71);
		generate_int_handler!(72);
		generate_int_handler!(73);
		generate_int_handler!(74);
		generate_int_handler!(75);
		generate_int_handler!(76);
		generate_int_handler!(77);
		generate_int_handler!(78);
		generate_int_handler!(79);
		generate_int_handler!(80);
		generate_int_handler!(81);
		generate_int_handler!(82);
		generate_int_handler!(83);
		generate_int_handler!(84);
		generate_int_handler!(85);
		generate_int_handler!(86);
		generate_int_handler!(87);
		generate_int_handler!(88);
		generate_int_handler!(89);
		generate_int_handler!(90);
		generate_int_handler!(91);
		generate_int_handler!(92);
		generate_int_handler!(93);
		generate_int_handler!(94);
		generate_int_handler!(95);
		generate_int_handler!(96);
		generate_int_handler!(97);
		generate_int_handler!(98);
		generate_int_handler!(99);
		generate_int_handler!(100);
		generate_int_handler!(101);
		generate_int_handler!(102);
		generate_int_handler!(103);
		generate_int_handler!(104);
		generate_int_handler!(105);
		generate_int_handler!(106);
		generate_int_handler!(107);
		generate_int_handler!(108);
		generate_int_handler!(109);
		generate_int_handler!(110);
		generate_int_handler!(111);
		generate_int_handler!(112);
		generate_int_handler!(113);
		generate_int_handler!(114);
		generate_int_handler!(115);
		generate_int_handler!(116);
		generate_int_handler!(117);
		generate_int_handler!(118);
		generate_int_handler!(119);
		generate_int_handler!(120);
		generate_int_handler!(121);
		generate_int_handler!(122);
		generate_int_handler!(123);
		generate_int_handler!(124);
		generate_int_handler!(125);
		generate_int_handler!(126);
		generate_int_handler!(127);
		generate_int_handler!(128);
		generate_int_handler!(129);
		generate_int_handler!(130);
		generate_int_handler!(131);
		generate_int_handler!(132);
		generate_int_handler!(133);
		generate_int_handler!(134);
		generate_int_handler!(135);
		generate_int_handler!(136);
		generate_int_handler!(137);
		generate_int_handler!(138);
		generate_int_handler!(139);
		generate_int_handler!(140);
		generate_int_handler!(141);
		generate_int_handler!(142);
		generate_int_handler!(143);
		generate_int_handler!(144);
		generate_int_handler!(145);
		generate_int_handler!(146);
		generate_int_handler!(147);
		generate_int_handler!(148);
		generate_int_handler!(149);
		generate_int_handler!(150);
		generate_int_handler!(151);
		generate_int_handler!(152);
		generate_int_handler!(153);
		generate_int_handler!(154);
		generate_int_handler!(155);
		generate_int_handler!(156);
		generate_int_handler!(157);
		generate_int_handler!(158);
		generate_int_handler!(159);
		generate_int_handler!(160);
		generate_int_handler!(161);
		generate_int_handler!(162);
		generate_int_handler!(163);
		generate_int_handler!(164);
		generate_int_handler!(165);
		generate_int_handler!(166);
		generate_int_handler!(167);
		generate_int_handler!(168);
		generate_int_handler!(169);
		generate_int_handler!(170);
		generate_int_handler!(171);
		generate_int_handler!(172);
		generate_int_handler!(173);
		generate_int_handler!(174);
		generate_int_handler!(175);
		generate_int_handler!(176);
		generate_int_handler!(177);
		generate_int_handler!(178);
		generate_int_handler!(179);
		generate_int_handler!(180);
		generate_int_handler!(181);
		generate_int_handler!(182);
		generate_int_handler!(183);
		generate_int_handler!(184);
		generate_int_handler!(185);
		generate_int_handler!(186);
		generate_int_handler!(187);
		generate_int_handler!(188);
		generate_int_handler!(189);
		generate_int_handler!(190);
		generate_int_handler!(191);
		generate_int_handler!(192);
		generate_int_handler!(193);
		generate_int_handler!(194);
		generate_int_handler!(195);
		generate_int_handler!(196);
		generate_int_handler!(197);
		generate_int_handler!(198);
		generate_int_handler!(199);
		generate_int_handler!(200);
		generate_int_handler!(201);
		generate_int_handler!(202);
		generate_int_handler!(203);
		generate_int_handler!(204);
		generate_int_handler!(205);
		generate_int_handler!(206);
		generate_int_handler!(207);
		generate_int_handler!(208);
		generate_int_handler!(209);
		generate_int_handler!(210);
		generate_int_handler!(211);
		generate_int_handler!(212);
		generate_int_handler!(213);
		generate_int_handler!(214);
		generate_int_handler!(215);
		generate_int_handler!(216);
		generate_int_handler!(217);
		generate_int_handler!(218);
		generate_int_handler!(219);
		generate_int_handler!(220);
		generate_int_handler!(221);
		generate_int_handler!(222);
		generate_int_handler!(223);
		generate_int_handler!(224);
		generate_int_handler!(225);
		generate_int_handler!(226);
		generate_int_handler!(227);
		generate_int_handler!(228);
		generate_int_handler!(229);
		generate_int_handler!(230);
		generate_int_handler!(231);
		generate_int_handler!(232);
		generate_int_handler!(233);
		generate_int_handler!(234);
		generate_int_handler!(235);
		generate_int_handler!(236);
		generate_int_handler!(237);
		generate_int_handler!(238);
		generate_int_handler!(239);
		generate_int_handler!(240);
		generate_int_handler!(241);
		generate_int_handler!(242);
		generate_int_handler!(243);
		generate_int_handler!(244);
		generate_int_handler!(245);
		generate_int_handler!(246);
		generate_int_handler!(247);
		generate_int_handler!(248);
		generate_int_handler!(249);
		generate_int_handler!(250);
		generate_int_handler!(251);
		generate_int_handler!(252);
		generate_int_handler!(253);
		generate_int_handler!(254);
		generate_int_handler!(255);
	}
}

/*
static mut test:TestInt = TestInt;

struct TestInt;

impl FnOnce<()> for TestInt {
	type Output = ();

	extern "rust-call" fn call_once(self, args: ()) -> Self::Output
	{
		unsafe {vga_println!("Called once?")};
		()
	}
}

impl FnMut<()> for TestInt {
	extern "rust-call" fn call_mut(&mut self, args: ()) -> ()
	{
		unsafe {
			let scan = io::inb(PS2_INOUT);

//		io::outb(PS2_INOUT, 0xF4);
//		let response = io::inb(PS2_INOUT);

//		vga_println!("response is {:x}", response);
		//vga_println!("got key {} ", scan);

		//io::outb(0x61, key | 0x80);
		//io::outb(0x61, key);
		//vga_println!("got key interrupt {:x}", scan);

		if (scan == 0xF0) {
			let key = io::inb(PS2_INOUT);
			vga_println!("released {:x}", key);
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
		()
	}
}

#[no_mangle]
pub extern "C" fn rust_int_unused()
{
	unsafe {
		vga_println!("I got an unknown interrupt");
		//for x in 0..32 {
		pic::end_interrupt(0);
		//}
	}
}


#[inline(never)]
#[no_mangle]
pub extern "C" fn rust_int_keyboard()
{
	unsafe {
		let scan = io::inb(PS2_INOUT);

//		io::outb(PS2_INOUT, 0xF4);
//		let response = io::inb(PS2_INOUT);

//		vga_println!("response is {:x}", response);
		//vga_println!("got key {} ", scan);

		//io::outb(0x61, key | 0x80);
		//io::outb(0x61, key);
		//vga_println!("got key interrupt {:x}", scan);

		if (scan == 0xF0) {
			let key = io::inb(PS2_INOUT);
			vga_println!("released {:x}", key);
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
		pic::end_interrupt(1)
	}
}

///the used asm functions
extern "C" {
	static mut int_unused: Label;
	static mut int_keyboard: Label;
}

*/










/*A
1C
F0,1C
 
B
32
F0,32
 
C
21
F0,21
 
D
23
F0,23

E
24
F0,24
 
F
2B
F0,2B
 
G
34
F0,34
 
H
33
F0,33

I
43
F0,43

J
3B
F0,3B

K
42
F0,42
 

L
4B
F0,4B

M
3A
F0,3A
 

N
31
F0,31
 
O
44
F0,44
 
P
4D
F0,4D
 
Q
15
F0,15

R
2D
F0,2D

S
1B
F0,1B

T
2C
F0,2C

U
3C
F0,3C

V
2A
F0,2A
 
W
1D
F0,1D

X
22
F0,22
 
Y
35
F0,35

Z
1A
F0,1A*/