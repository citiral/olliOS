///This module defines a set of port input/output functions
///use them as you would any C or ASM port IO library
///
///functions suffixed with b expect an u8, those with w an u16 and l an u32


pub unsafe fn outb(port: u16, value: u8)
{
	asm!("outb %al, %dx"::"{ax}" (value), "{dx}" (port) );
}

pub unsafe fn outw(port: u16, value: u16)
{
	asm!("outw %ax, %dx"::"{ax}" (value), "{dx}" (port) );
}

pub unsafe fn outl(port: u16, value: u32)
{
	asm!("outl %eax, %dx"::"{eax}" (value), "{dx}" (port) );
}

pub unsafe fn inb(port: u16) -> u8
{
	let mut value: u8;
	asm!("inb %dx, %al":"={ax}" (value):"{dx}" (port));
	value
}

pub unsafe fn inw(port: u16) -> u16
{
	let mut value: u16;
	asm!("inw %dx, %ax":"={ax}" (value):"{dx}" (port));
	value
}


pub unsafe fn inl(port: u16) -> u16
{
	let mut value: u16;
	asm!("inl %dx, %eax":"={eax}" (value):"{dx}" (port));
	value
}
