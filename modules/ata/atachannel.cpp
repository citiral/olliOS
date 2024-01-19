#include "atachannel.h"
#include "cpu/io.h"

using namespace ata;


AtaChannel::AtaChannel(u32 base, u32 ctrl, u32 irq): base(base), ctrl(ctrl), irq(irq), _selected_drive(AtaDrive::Unknown), _lock(), interrupted(false)
{
}

u8 AtaChannel::read_u8(AtaDrive drive, AtaRegister reg)
{
    select_drive(drive);

    if (reg < AltStatus) {
        return inb(base + reg);
    } else {
        return inb(ctrl + (reg - 0xF0));
    }
}

void AtaChannel::write_u8(AtaDrive drive, AtaRegister reg, u8 data)
{
    select_drive(drive);
    
    if (reg < AltStatus) {
        outb(base + reg, data);
    } else {
        outb(ctrl + (reg - 0xF0), data);
    }
}

u16 AtaChannel::read_u16(AtaDrive drive, AtaRegister reg)
{
    select_drive(drive);

    if (reg < AltStatus) {
        return inw(base + reg);
    } else {
        return inw(ctrl + (reg - 0xF0));
    }
}

void AtaChannel::write_u16(AtaDrive drive, AtaRegister reg, u16 data)
{
    select_drive(drive);
    
    if (reg < AltStatus) {
        outw(base + (u16) reg, data);
    } else {
        outw(ctrl + ((u16)reg - 0xF0), data);
    }
}

void AtaChannel::lock()
{
    _lock.lock();
}

void AtaChannel::release()
{
    _lock.release();
}

void AtaChannel::select_drive(AtaDrive drive)
{
    if (_selected_drive != drive) {
        _selected_drive = drive;
        
        if (drive == AtaDrive::Master) {
            outb(base + AtaRegister::Hddevsel, 0b11100000);
        } else {
            outb(base + AtaRegister::Hddevsel, 0b11110000);
        }

		// waste a bit of time to make sure the drive select has gone through
		// reading the status register 5 times equals to around 500ns which should be enough (we are expected to wait around 400ns)
        for (int i = 0 ; i < 5 ; i++) {
		    inb(base + AtaRegister::Status);
        }
    }
}
