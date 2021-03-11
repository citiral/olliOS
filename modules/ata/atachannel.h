//
// Created by Olivier on 24/09/16.
//

#ifndef OLLIOS_GIT_ATACHANNEL_H
#define OLLIOS_GIT_ATACHANNEL_H

#include "threading/mutex.h"
#include "types.h"

namespace ata {

enum AtaRegister {
    Data = 0,
    Error = 1,
    Features = 1,
    Seccount = 2,
    Lba0 = 3,
    Lba1 = 4,
    Lba2 = 5,
    Hddevsel = 6,
    Command = 7,
    Status = 7,
    AltStatus = 0xF2,
    Control = 0xF2,
    Devaddress = 0xF3,
};

enum AtaDrive {
    Unknown = -1,
    Master = 0,
    Slave = 1,
};

class AtaChannel {
public:
    AtaChannel(u32 base, u32 ctrl, u32 irq);

    u8 read_u8(AtaDrive drive, AtaRegister reg);
    void write_u8(AtaDrive drive, AtaRegister reg, u8 data);
    u16 read_u16(AtaDrive drive, AtaRegister reg);
    void write_u16(AtaDrive drive, AtaRegister reg, u16 data);

    void lock();
    void release();

    u32 base;
    u32 ctrl;
    u32 irq;

private:
    void select_drive(AtaDrive drive);

    AtaDrive _selected_drive;
    threading::Mutex _lock;
};

}

#endif //OLLIOS_GIT_ATA_H
