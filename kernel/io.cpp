///This module defines a set of port input/output functions
///use them as you would any C or ASM port IO library
///
///functions suffixed with b expect an u8, those with w an u16 and l an u32

#include "io.h"

void outb(u16 port, u8 value)
{
	__asm__ volatile("outb %0, %1"
		:
		:"a" (value), "dN" (port));
}

void outw(u16 port, u16 value)
{
	__asm__ volatile("outw %0, %1"
		:
		:"a" (value), "dN" (port));
}

void outl(u16 port, u32 value)
{
	__asm__ volatile("outl %0, %1"
		:
		:"a" (value), "dN" (port) );
}

u8 inb(u16 port)
{
	u8 value;
	__asm__ volatile("inb %1, %0"
		:"=a" (value)
		:"dN" (port));
	return value;
}
u16 inw(u16 port)
{
	u16 value;
	__asm__ volatile("inw %1, %0"
		:"=a" (value)
		:"dN" (port));
	return value;
}

u32 inl(u16 port)
{
	u32 value;
	__asm__ volatile("inl %1, %0"
		:"=a" (value)
		:"dN" (port));
	return value;
}
