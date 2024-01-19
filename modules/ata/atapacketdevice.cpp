//
// Created by Olivier on 30/09/16.
//

#include <stdio.h>
#include "cdefs.h"
#include "cpu/io.h"
#include "ata.h"
#include "atapacketdevice.h"
#include "threading/thread.h"

namespace ata {

AtaPacketDevice::AtaPacketDevice(fs::File* ata, AtaChannel* channel, AtaDrive drive, unsigned short* identify_data, u32 deviceId): AtaDevice(ata, channel, drive, identify_data, deviceId) {
}

AtaPacketDevice::~AtaPacketDevice() {
}

size_t AtaPacketDevice::read(void* data, size_t amount, size_t offset) {
    // TODO it seems to get stuck when issueing very big reads
    channel->interrupted = false;
    
    // round offset down, add count removed from offset to amount, and round amount up.
    size_t actual_amount = amount;
    size_t skip = offset % 2048;
    actual_amount += skip;
    offset -= skip;
    if (actual_amount % 2048 != 0)
        actual_amount += 2048 - (actual_amount % 2048);

    // we are not going to use DMA (set dma bit to zero)
    channel->write_u8(drive, AtaRegister::Features, 0);
    //outb(_port+PORT_FEATURE, 0);

    // set the max amount of bytes we want to receive
    channel->write_u8(drive, AtaRegister::Lba1, actual_amount & 0xFF);
    channel->write_u8(drive, AtaRegister::Lba2, actual_amount >> 8);
    for(int i = 0; i < 40; i++) {
        channel->read_u8(drive,AtaRegister::Status);
    }

    // convert the actual_amount and offset to blocks
    actual_amount /= 2048;
    offset /= 2048;

    // send the packet command
    channel->write_u8(drive, AtaRegister::Command, AtaCommand::Packet);

    waitForBusy();
    if (!waitForDataOrError())
        return 0;

    // generate the commands we are going to send
    unsigned char commands[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // the operation code (read 12)
    commands[0] = 0xA8;

    // ignore this protection information
    commands[1] = 0;

    // send the lba MSB first
    commands[2] = offset >> 24;
    commands[3] = offset >> 16;
    commands[4] = offset >> 8;
    commands[5] = offset;

    // send the transfer length (in blocks) MSB first
    commands[6] = actual_amount >> 24;
    commands[7] = actual_amount >> 16;
    commands[8] = actual_amount >> 8;
    commands[9] = actual_amount;

    // set the group number to 0, I presume we can ignore this
    commands[10] = 0;

    // I presume the control byte can be zero
    commands[11] = 0;

    // now lets send the commands
    outsw(channel->base + AtaRegister::Data, commands, 6);
    // wait until the device is ready
    waitForBusy();
    //waitForInterrupt();
    if (!waitForDataOrError())
        return 0;

    // now lets fetch the actual_amount of data we can read
    u8 hi = channel->read_u8(drive, AtaRegister::Lba2);
    u8 lo = channel->read_u8(drive, AtaRegister::Lba1);
    size_t wordcount = (((size_t)hi) << 8) + lo;

    // and now finally we can read the data
    for (size_t i = 0 ; i < wordcount ; i += 2) {
        u16 byte = channel->read_u16(drive, AtaRegister::Data);
        u8* read = (u8*)&byte;

        for (size_t k = 0 ; k < 2 ; k++) {
            if ((i + k) >= skip && (i + k) < (skip + amount)) {
                ((u8*)data)[i + k - skip] = read[k];
            }
        }
    }
    // the device is going to send us one last interrupt
    //waitForNoDataOrError();

    return (wordcount < amount ? wordcount : amount);
}

size_t AtaPacketDevice::write(const void* data, size_t amount, size_t offset)
{
    /// This might not work, I find no documentation about atapi writing so this could be bogus

    // TODO it seems to get stuck when issueing very big reads
    channel->interrupted = false;
    
    // round offset down, add count removed from offset to amount, and round amount up.
    size_t actual_amount = amount;
    size_t skip = offset % 2048;
    actual_amount += skip;
    offset -= skip;
    if (actual_amount % 2048 != 0)
        actual_amount += 2048 - (actual_amount % 2048);

    // we are not going to use DMA (set dma bit to zero)
    channel->write_u8(drive, AtaRegister::Features, 0);
    //outb(_port+PORT_FEATURE, 0);

    // set the max amount of bytes we want to receive
    channel->write_u8(drive, AtaRegister::Lba1, actual_amount & 0xFF);
    channel->write_u8(drive, AtaRegister::Lba2, actual_amount >> 8);
    for(int i = 0; i < 40; i++) {
        channel->read_u8(drive,AtaRegister::Status);
    }

    // convert the actual_amount and offset to blocks
    actual_amount /= 2048;
    offset /= 2048;

    // send the packet command
    channel->write_u8(drive, AtaRegister::Command, AtaCommand::Packet);

    waitForBusy();
    if (!waitForDataOrError())
        return 0;

    // generate the commands we are going to send
    unsigned char commands[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // the operation code (write 12)
    commands[0] = 0xAA;

    // ignore this protection information
    commands[1] = 0;

    // send the lba MSB first
    commands[2] = offset >> 24;
    commands[3] = offset >> 16;
    commands[4] = offset >> 8;
    commands[5] = offset;

    // send the transfer length (in blocks) MSB first
    commands[6] = actual_amount >> 24;
    commands[7] = actual_amount >> 16;
    commands[8] = actual_amount >> 8;
    commands[9] = actual_amount;

    // set the group number to 0, I presume we can ignore this
    commands[10] = 0;

    // I presume the control byte can be zero
    commands[11] = 0;

    // now lets send the commands
    outsw(channel->base + AtaRegister::Data, commands, 6);
    // wait until the device is ready
    waitForBusy();
    //waitForInterrupt();
    if (!waitForDataOrError())
        return 0;

    // now lets fetch the actual_amount of data we can read
    u8 hi = channel->read_u8(drive, AtaRegister::Lba2);
    u8 lo = channel->read_u8(drive, AtaRegister::Lba1);
    size_t wordcount = (((size_t)hi) << 8) + lo;

    // and now finally we can write the data
    for (size_t i = 0 ; i < wordcount ; i += 2) {
        for (size_t k = 0 ; k < 2 ; k++) {
            if ((i + k) >= skip && (i + k) < (skip + amount)) {
                channel->write_u16(drive, AtaRegister::Data, ((u8*)data)[i + k - skip]);
            } else {
                channel->write_u16(drive, AtaRegister::Data, 'A');
            }
        }
    }
    // the device is going to send us one last interrupt
    waitForNoDataOrError();

    return (wordcount < amount ? wordcount : amount);
}

}