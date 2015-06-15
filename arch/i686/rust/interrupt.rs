#[allow(unused_imports)]
use prelude::*;
use descriptor;
use core::fmt::Write;
use pic;
use types::Label;
use io;
use core::raw::{self, Repr};

const PS2_INOUT: u16 = 0x60;
const PS2_RW: u16 = 0x64;

const MAX_IDT_ENTRIES: usize = 256;

pub static mut IDT: Idt<'static> = Idt {
	table: [IdtDescriptor {
		offset_low: 0,
		selector: 0,
		zero: 0,
		type_attr: 0,
		offset_hi: 0,
	};MAX_IDT_ENTRIES],
	callbacks: [None; MAX_IDT_ENTRIES],
	length: 0
};

pub struct Idt<'a> {
	///The table containing the gdt descriptors
	table: [IdtDescriptor;MAX_IDT_ENTRIES],
	///the list of callbacks linked to the interrupts
	callbacks: [Option<&'a FnMut()>; MAX_IDT_ENTRIES],
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
			callbacks: [None; MAX_IDT_ENTRIES],
			length: 0
		}
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

#[no_mangle]
///registers all interrupts to the IDT
pub unsafe extern "C"  fn register_interrupts()
{

	let address: u32;

	asm!("
		jmp 2f
	1:
		pushal
		cld
		call rust_int_test
		popal
		iret
	2:
	leal 1b, %eax":"={eax}" (address):::"volatile");


	/*unsafe {
		//first, set all addresses to unused
		for x in 0..256 {
			IDT.get_entry(x).set_offset(address);
		}
		//IDT.get_entry(0x21).set_offset(label_addr!(int_keyboard) as u32);
	}*/


}

#[no_mangle]
pub extern "C" fn rust_int_test()
{
	unsafe {
		vga_println!("I got a test int!");
		//for x in 0..32 {
		pic::end_interrupt(0);
		//}
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
	fn reload_idt(limit: u16, base: u32);
	static mut int_unused: Label;
	static mut int_keyboard: Label;
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