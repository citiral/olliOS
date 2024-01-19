
#include <stdio.h>
#include "cdefs.h"
#include "cpu/io.h"
#include "ata.h"
#include "atapiodevice.h"
#include "threading/thread.h"

namespace ata {
AtaPioDevice::AtaPioDevice(fs::File* ata, AtaChannel* channel, AtaDrive drive, unsigned short* identify_data, u32 deviceId): AtaDevice(ata, channel, drive, identify_data, deviceId) {

}

AtaPioDevice::~AtaPioDevice() {

}


size_t AtaPioDevice::read(void* data, size_t amount, size_t offset) {
    size_t sector_count = (amount + 511) / 512;
    size_t sectors_offset = offset / 512;
    size_t offset_in_first_sector = offset % 512;
    
    if (offset_in_first_sector > 0)
        sector_count += 1;

    // we can only read up to 256 sectors at a time, and a sector count of 0 represents 256 sectors.
    if (sector_count >= 256) {
        sector_count = 256;
        channel->write_u8(drive, AtaRegister::Seccount, 0);
    } else {
        channel->write_u8(drive, AtaRegister::Seccount, sector_count);
    }

    channel->write_u8(drive, AtaRegister::Lba0, sectors_offset);
    channel->write_u8(drive, AtaRegister::Lba1, sectors_offset >> 8);
    channel->write_u8(drive, AtaRegister::Lba2, sectors_offset >> 16);
    
    channel->write_u8(drive, AtaRegister::Command, 0x20); // READ command

    size_t i = 0;
    size_t read_bytes = 0;
    while (sector_count > 0) {
        sector_count -= 1;

        for (volatile int j = 0 ; j < 10000000; j++);
        waitForBusy();
        if (!waitForDataOrError()) {
            return 0;
        }

        for (int j = 0 ; j < 256 ; j++) {
            u16 byte = channel->read_u16(drive, AtaRegister::Data);
            u8* read = (u8*)&byte;

            for (size_t k = 0 ; k < 2 ; k++) {
                if (i >= offset_in_first_sector && i - offset_in_first_sector < amount) {
                    ((u8*)data)[i - offset_in_first_sector] = read[k];
                    read_bytes += 1;
                }
            
                i++;
            }
        }
    }

    return read_bytes;
}

size_t AtaPioDevice::write(const void* data, size_t amount, size_t offset) {
    if (amount % 2 == 1) {
        amount -= 1;
    }

    size_t sector_count = (amount + 511) / 512;
    size_t sectors_offset = offset / 512;
    size_t offset_in_first_sector = offset % 512;
    
    if (offset_in_first_sector > 0)
        sector_count += 1;

    // we can only read up to 256 sectors at a time, and a sector count of 0 represents 256 sectors.
    if (sector_count >= 256) {
        sector_count = 256;
        channel->write_u8(drive, AtaRegister::Seccount, 0);
    } else {
        channel->write_u8(drive, AtaRegister::Seccount, sector_count);
    }

    channel->write_u8(drive, AtaRegister::Lba0, sectors_offset);
    channel->write_u8(drive, AtaRegister::Lba1, sectors_offset >> 8);
    channel->write_u8(drive, AtaRegister::Lba2, sectors_offset >> 16);
    
    channel->write_u8(drive, AtaRegister::Command, 0x30); // WRITE command

    size_t i = 0;
    size_t read_bytes = 0;
    while (sector_count > 0) {
        sector_count -= 1;

        for (volatile int j = 0 ; j < 10000000; j++);
        waitForBusy();
        if (!waitForDataOrError()) {
            return 0;
        }

        for (int j = 0 ; j < 256 ; j++) {
            u16 byte = 0;
            u8* write = (u8*)&byte;

            for (size_t k = 0 ; k < 2 ; k++) {
                if (i >= offset_in_first_sector && i - offset_in_first_sector < amount) {
                    write[k] = ((u8*)data)[i - offset_in_first_sector];
                }
            
                i++;
            }

            printf("%c%c", (char) byte, (char) (byte >> 8));
            channel->write_u16(drive, AtaRegister::Data, byte);
        }
    }

    waitForBusy();
    
    channel->write_u8(drive, AtaRegister::Command, 0xE7); // cache flush
    waitForBusy();

    return read_bytes;
}


}