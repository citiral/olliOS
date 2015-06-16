use io;
use core::fmt::Write;

const MASTER_COMMAND: u16 = 0x20;
const MASTER_DATA: u16 = 0x21;
const SLAVE_COMMAND: u16 = 0xA0;
const SLAVE_DATA: u16 = 0xA1;

const EOI: u8 = 0x20; //end of interrupt command for the PIC
const INIT: u8 = 0x11;
const ICW4_8086: u8 = 0x01;
///ends an interrupt comming from the PIC
pub unsafe fn end_interrupt(irq: u8)
{//
	//io::inb(SLAVE_COMMAND);
	//io::inb(MASTER_COMMAND);
	//if the irq is coming from the slave, send the command to him
	if irq >= 8 {
		io::outb(SLAVE_COMMAND, EOI);
	}
	//and send the command to the master
	io::outb(MASTER_COMMAND, EOI);	
}

///clears all interrupt masks of the PIC
pub unsafe fn clear_all_masks()
{
	io::outb(MASTER_DATA, 0);
	io::outb(SLAVE_DATA, 0);
}

pub unsafe fn enable_irq(irq: u8)
{
	let mut mask = if irq >= 8 {
		io::inb(SLAVE_DATA)
	} else {
		io::inb(MASTER_DATA)
	};
	vga_println!("old mask is {}", mask);
	mask = mask & (0xFFFF ^ (0x0001 << irq));
	vga_println!("new mask is {}", mask);
	
	if irq >= 8 {
		io::outb(SLAVE_DATA, mask)
	} else {
		io::outb(MASTER_DATA, mask)
	};	
}

pub unsafe fn disable_irq(irq: u8)
{
	let mut mask = if irq >= 8 {
		io::inb(SLAVE_DATA)
	} else {
		io::inb(MASTER_DATA)
	};
	vga_println!("old mask is {}", mask);
	mask = mask | (0x0001 << irq);
	vga_println!("new mask is {}", mask);
	
	if irq >= 8 {
		io::outb(SLAVE_DATA, mask)
	} else {
		io::outb(MASTER_DATA, mask)
	};	
}

pub unsafe fn map_pics(master_offset: u8, slave_offset: u8)
{
	//first get the old maps
	let a1 = io::inb(MASTER_DATA);
	let a2 = io::inb(SLAVE_DATA);

	//begin the initialisation
	io::outb(MASTER_COMMAND, INIT);
	io::outb(SLAVE_COMMAND, INIT);

	//send the offsets
	io::outb(MASTER_DATA, master_offset);
	io::outb(SLAVE_DATA, slave_offset);

	//tell master it has a slave
	io::outb(MASTER_DATA, 4);
	//tell slave the cascade identity(??)
	io::outb(SLAVE_DATA, 2);

	//and the last value that always needs to be send
	io::outb(MASTER_DATA, ICW4_8086);
	io::outb(SLAVE_DATA, ICW4_8086);

	//and restore the old masks
	io::outb(MASTER_DATA, a1);
	io::outb(SLAVE_DATA, a2);
}

///initializes the pic and allows all interrupts
///this also remaps the the pic so it doesn't conflict with intel reserved interrupts
pub unsafe fn init_pic()
{
	map_pics(0x20, 0x28);
	clear_all_masks();
	disable_irq(0);

		for x in 0..16 {
			end_interrupt(x as u8);
		}

	asm!("sti"::::"volatile");
}