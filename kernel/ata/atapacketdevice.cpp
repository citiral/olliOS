//
// Created by Olivier on 30/09/16.
//

#include <stdio.h>
#include "cdefs.h"
#include "io.h"
#include "ata/ata.h"
#include "ata/atapacketdevice.h"

AtaPacketDevice::AtaPacketDevice(u16 port, unsigned short* data, u8 drive): AtaDevice(port, data, drive), _lba(0) {
}

AtaPacketDevice::~AtaPacketDevice() {
    delete[] _data;
}

DeviceType AtaPacketDevice::getDeviceType() const {
    return DeviceType::Storage;
}

void AtaPacketDevice::getDeviceInfo(void* deviceinfo) const
{
	DeviceStorageInfo* info = (DeviceStorageInfo*)deviceinfo;
	info->deviceInfo.name = (char*)(_data + 27);
}

size_t AtaPacketDevice::read(void* data, size_t amount) {
    // select ourselves as drive
    ataDriver.selectDevice(_port, _drive);
    
    // round amount to the lowest block (2k boundary)
    amount -= amount % 2048;

    // we are not going to use DMA (set dma bit to zero)
    outb(_port+PORT_FEATURE, 0);

    // set the max amount of bytes we want to receive
    outb(_port+PORT_CYLINDER_LOW, amount & 0xFF);
    outb(_port+PORT_CYLINDER_HIGH, amount >> 8);

    // convert the amount to blocks
    amount /= 2048;

    // send the packet command
    outb(_port+PORT_COMMAND, COMMAND_PACKET);

    // generate the commands we are going to send
    unsigned char commands[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // the operation code (read 12)
    commands[0] = 0xA8;

    // ignore this protection information
    commands[1] = 0;

    // send the lba MSB first
    commands[2] = _lba >> 24;
    commands[3] = _lba >> 16;
    commands[4] = _lba >> 8;
    commands[5] = _lba;

    // send the transfer length (in blocks) MSB first
    commands[6] = amount >> 24;
    commands[7] = amount >> 16;
    commands[8] = amount >> 8;
    commands[9] = amount;

    // wait until the device is ready
    ataDriver.waitForBusy(_port);
    ataDriver.waitForDataOrError(_port);

    // set the group number to 0, I presume we can ignore this
    commands[10] = 0;

    // I presume the control byte can be zero
    commands[11] = 0;

    // now lets send the commands
    outsw(_port+PORT_DATA, commands, 6);

    // wait until the device is ready
    ataDriver.waitForInterrupt(_port);

    // now lets fetch the amount of data we can read
    int wordcount = ((inb(_port+PORT_LBA_HIGH) << 8) | inb(_port+PORT_LBA_MID)) / 2;

    // and now finally we can read the data
    for (int i = 0 ; i < wordcount ; i++) {
        ((unsigned short*)data)[i] = inw(_port+PORT_DATA);
    }

    // the device is going to send us one last interrupt
    ataDriver.waitForBusy(_port);
    ataDriver.waitForInterrupt(_port);

    // advance the lba by wordcount / 1024 (advance once for each 2K block read)
    _lba += wordcount / 1024;

    return wordcount * 2;
}

size_t AtaPacketDevice::write(const void* data, size_t amount)
{
	UNUSED(data);
	UNUSED(amount);
	CPU::panic("write not implemented on AtaPacketDevice");
	return 0;
}

size_t AtaPacketDevice::write(const void* data)
{
	UNUSED(data);
	CPU::panic("write not implemented on AtaPacketDevice");
	return 0;
}

size_t AtaPacketDevice::write(char data)
{
	UNUSED(data);
	CPU::panic("write not implemented on AtaPacketDevice");
	return 0;
}

size_t AtaPacketDevice::seek(i32 offset, int position) {
    // we only accept 2kb boundaries
    if (offset % 2048 != 0)
        return 1;

    offset /= 2048;

    if (position == SEEK_SET)
        _lba = offset;
    else if (position == SEEK_CUR)
        _lba += offset;
    else if (position == SEEK_END)
        LOG_UNIMPLEMENTED();

    return 0;
}