#ifndef __IO_H
#define __IO_H

///This module defines a set of port input/output functions
///use them as you would any C or ASM port IO library
///
///functions suffixed with b expect an u8, those with w an u16 and l an u32

#include "kstd/cstddef.h"
#include "types.h"

void outb(u16 port, u8 value);
void outw(u16 port, u16 value);
void outl(u16 port, u32 value);
void outsw (unsigned short int port, const void *addr, unsigned long int count);

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);
void insw (unsigned short int port, void *addr, unsigned long int count);

#endif /* end of include guard: __IO_H */
