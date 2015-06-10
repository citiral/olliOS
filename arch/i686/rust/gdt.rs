use descriptor::{self, Dpl};
use core::raw::{self, Repr};
use core::fmt::Write;

const MAX_GDT_ENTRIES: usize = 255;

pub struct Gdt {
	///The table containing the gdt descriptors
	table: [GdtDescriptor;MAX_GDT_ENTRIES],
	///amount of used descriptors in the table
	length: usize,
}

impl Gdt {
	///generates a new gdt filled with empty entries
	pub fn new() -> Gdt {
		Gdt {
			table: [GdtDescriptor {
				limit: 0,
				base_low: 0,
				base_mid: 0,
				access: 16,
				limit_flags: 0,
				base_hi: 0,
			};MAX_GDT_ENTRIES],
			length: 0
		}
	}

	pub fn get_entry(&mut self, index: usize) -> &mut GdtDescriptor
	{
		&mut self.table[index]
	}

	pub fn set_entry(&mut self, index: usize, entry: GdtDescriptor)
	{
		self.table[index] = entry;
	}

	pub fn add_entry(&mut self, entry: GdtDescriptor)
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

	///Flushes the GDT. Any change made to used GDT entries will be reflected as soon
	///as the segment registers are reloaded, but if the GDT is resized flush needs to be called.
	///Flushing the gdt also makes sure the segment registers are reloaded.
	pub unsafe fn flush(&self)
	{
		let pointer = self.generate_table_pointer();
		let answer = reload_gdt(pointer.limit, pointer.base);
	}

	///generates a table descriptor pointer for this gdt
	fn generate_table_pointer(&self) -> descriptor::DescriptorTablePointer
	{
		let tableptr: raw::Slice<GdtDescriptor> = self.table.repr();
		descriptor::DescriptorTablePointer {
			base: tableptr.data as u32,
			limit: (self.length*8) as u16,
		}
	}
}

#[repr(C, packed)]
#[derive(Copy, Clone, Debug)]
pub struct GdtDescriptor {
	limit: u16,
	base_low: u16,
	base_mid: u8,
	access: u8,
	limit_flags: u8,
	base_hi: u8,
}

impl GdtDescriptor {
	fn new() -> GdtDescriptor
	{
		GdtDescriptor {
			limit: 0,
			base_low: 0,
			base_mid: 0,
			access: 16,
			limit_flags: 0,
			base_hi: 0,
		}
	}

	fn from_value(lower: u32, higher: u32) -> GdtDescriptor
	{
		GdtDescriptor {
			limit: lower as u16,
			base_low: (lower >> 16) as u16,
			base_mid: (higher) as u8,
			access: (higher >> 8) as u8,
			limit_flags: (higher >> 16) as u8,
			base_hi: (higher >> 24) as u8,
		}
	}

	///sets the limit of the Descriptor. The value is an u32, but only the lower 20 bits are used
	pub fn set_limit(&mut self, value: u32)
	{
		self.limit = value as u16;
		self.limit_flags = (self.limit_flags & 0b11110000) | (((value >> 16) as u8) & 0b00001111);
	}

	pub fn set_base(&mut self, value: u32)
	{
		self.base_low = value as u16;
		self.base_mid = (value >> 16) as u8;
		self.base_hi = (value >> 24) as u8;
	}

	pub fn set_present(&mut self, present: bool)
	{
		self.access = (self.access & 0b01111111) | ((present as u8) << 7); 
	}

	pub fn set_dpl(&mut self, dpl: Dpl)
	{
		let value = dpl as u8;
		self.access = (self.access & 0b10011111) | value << 5;
	}

	///sets the exectuable bit true = 1 = is executable, false = 0 = data
	pub fn set_executable(&mut self, exectuable: bool)
	{
		self.access = (self.access & 0b11110111) | ((exectuable as u8) << 3); 
	}

	///sets the direction bit
	pub fn set_direction(&mut self, up: bool)
	{
		self.access = (self.access & 0b11111011) | ((up as u8) << 2); 
	}

	pub fn set_rw(&mut self, writeable: bool)
	{
		self.access = (self.access & 0b11111101) | ((writeable as u8) << 1); 
	}

	///if the page is true the granularity is set to page granularity (4KB) otherwise it is in bytes
	pub fn set_granularity(&mut self, page: bool)
	{
		self.limit_flags = (self.limit_flags & 0b01111111) | ((page as u8) << 7);
	}

	///if 0 this is a 16 bit selector, else it is a 32 bit selector
	pub fn set_size(&mut self, size: bool)
	{
		self.limit_flags = (self.limit_flags & 0b10111111) | ((size as u8) << 6);
	}

	//directly set the access flags
	pub fn set_type(&mut self, access: u8)
	{
		self.access = access;
	}
}

pub fn create_flat_gdt() -> Gdt
{
	let mut gdt = Gdt::new();
	gdt.add_entry(GdtDescriptor::from_value(0x00000000, 0x00000000));//NULL entry
	gdt.add_entry(GdtDescriptor::from_value(0x0000FFFF, 0x00CF9A00));//PL0 Code
	gdt.add_entry(GdtDescriptor::from_value(0x0000FFFF, 0x00CF9200));//PL0 Data
	gdt.add_entry(GdtDescriptor::from_value(0x0000FFFF, 0x00CFFA00));//PL3 Code
	gdt.add_entry(GdtDescriptor::from_value(0x0000FFFF, 0x00CFF200));//PL3 Data
	gdt.add_entry(GdtDescriptor::from_value(0x00000068, 0x00408900));//TSS entry
	gdt
}

extern "C" {
	fn reload_gdt(limit: u16, base: u32);
}

pub fn assert_correctness()
{
	assert_size!(GdtDescriptor, 8);
}