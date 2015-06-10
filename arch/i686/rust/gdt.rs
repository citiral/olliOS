use descriptor::{self, Dpl};

const MAX_GDT_ENTRIES: usize = 255;

static mut GDT: [GdtDescriptor;MAX_GDT_ENTRIES] = [GdtDescriptor {
	limit: 0,
	base_low: 0,
	base_mid: 0,
	access: 16,
	limit_flags: 0,
	base_hi: 0,
};MAX_GDT_ENTRIES];

#[repr(C, packed)]
#[derive(Copy, Clone, Debug)]
struct GdtDescriptor {
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
			limit_flags: (higher >> 8) as u8,
			base_hi: (higher >> 8) as u8,
		}
	}

	///sets the limit of the Descriptor. The value is an u32, but only the lower 20 bits are used
	fn set_limit(&mut self, value: u32)
	{
		self.limit = value as u16;
		self.limit_flags = (self.limit_flags & 0b11110000) | (((value >> 16) as u8) & 0b00001111);
	}

	fn set_base(&mut self, value: u32)
	{
		self.base_low = value as u16;
		self.base_mid = (value >> 16) as u8;
		self.base_hi = (value >> 24) as u8;
	}

	fn set_present(&mut self, present: bool)
	{
		self.access = (self.access & 0b01111111) | ((present as u8) << 7); 
	}

	fn set_dpl(&mut self, dpl: Dpl)
	{
		let value = dpl as u8;
		self.access = (self.access & 0b10011111) | value << 5;
	}

	///sets the exectuable bit true = 1 = is executable, false = 0 = data
	fn set_executable(&mut self, exectuable: bool)
	{
		self.access = (self.access & 0b11110111) | ((exectuable as u8) << 3); 
	}

	///sets the direction bit
	fn set_direction(&mut self, up: bool)
	{
		self.access = (self.access & 0b11111011) | ((up as u8) << 2); 
	}

	fn set_rw(&mut self, writeable: bool)
	{
		self.access = (self.access & 0b11111101) | ((writeable as u8) << 1); 
	}

	///if the page is true the granularity is set to page granularity (4KB) otherwise it is in bytes
	fn set_granularity(&mut self, page: bool)
	{
		self.limit_flags = (self.limit_flags & 0b01111111) | ((page as u8) << 7);
	}

	///if 0 this is a 16 bit selector, else it is a 32 bit selector
	fn set_size(&mut self, size: bool)
	{
		self.limit_flags = (self.limit_flags & 0b10111111) | ((size as u8) << 6);
	}

	//directly set the access flags
	fn set_type(&mut self, access: u8)
	{
		self.access = access;
	}
}

pub unsafe fn init_flat_gdt()
{
	GDT[0] = GdtDescriptor::from_value(0x00000000, 0x00000000);//NULL entry
	GDT[1] = GdtDescriptor::from_value(0x0000FFFF, 0x00CF9A00);//PL0 Code
	GDT[2] = GdtDescriptor::from_value(0x0000FFFF, 0x00CF9200);//PL0 Data
	GDT[3] = GdtDescriptor::from_value(0x0000FFFF, 0x00CFFA00);//PL3 Code
	GDT[4] = GdtDescriptor::from_value(0x0000FFFF, 0x00CFF200);//PL3 Data
	GDT[5] = GdtDescriptor::from_value(0x00000068, 0x00408900);//TSS entry
}

pub fn assert_correctness()
{
	assert_size!(GdtDescriptor, 8);
}