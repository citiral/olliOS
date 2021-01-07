//
// Created by Olivier on 30/09/16.
//

#include <stdio.h>
#include "cdefs.h"
#include "cpu/io.h"
#include "ata.h"
#include "atapacketdevice.h"

namespace ata {

AtaPacketDevice::AtaPacketDevice(fs::File* ata, u16 port, unsigned short* data, u8 drive): AtaDevice(ata, port, data, drive) {
}

AtaPacketDevice::~AtaPacketDevice() {
}

size_t AtaPacketDevice::read(void* data, size_t amount) {
    driver.grab();
    driver.clearInterruptFlag();
    
    // select ourselves as drive
    driver.selectDevice(_port, _drive);
    outb(_port + PORT_DRIVE, 0 << 4);
    
    // round offset down, add count removed from offset to amount, and round amount up.
    size_t offset = _pointer;
    size_t actual_amount = amount;
    size_t skip = offset % 2048;
    actual_amount += skip;
    offset -= skip;
    if (actual_amount % 2048 != 0)
        actual_amount += 2048 - (actual_amount % 2048);
    _pointer += amount;

    // we are not going to use DMA (set dma bit to zero)
    outb(_port+PORT_FEATURE, 0);

    // set the max amount of bytes we want to receive
    outb(_port+PORT_CYLINDER_LOW, actual_amount & 0xFF);
    outb(_port+PORT_CYLINDER_HIGH, actual_amount >> 8);

   for(int i = 0; i < 40; i++)
       inb(_port + PORT_STATUS); // Reading the Alternate Status port wastes 100ns.

    // convert the actual_amount and offset to blocks
    actual_amount /= 2048;
    offset /= 2048;

    // send the packet command
    outb(_port+PORT_COMMAND, COMMAND_PACKET);

    //driver.waitForInterrupt(_port);
    driver.waitForBusy(_port);
    driver.waitForDataOrError(_port);

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
    outsw(_port+PORT_DATA, commands, 6);
    // wait until the device is ready
    //driver.waitForInterrupt(_port);
    driver.waitForBusy(_port);
    //driver.waitForInterrupt(_port);

    // now lets fetch the actual_amount of data we can read
    size_t wordcount = ((inb(_port+PORT_LBA_HIGH) << 8) | inb(_port+PORT_LBA_MID));
    
    // and now finally we can read the data
    for (size_t i = 0 ; i < wordcount ; i += 2) {
        u16 byte = inw(_port+PORT_DATA);
        u8* read = (u8*)&byte;

        for (size_t k = 0 ; k < 2 ; k++) {
            if ((i + k) >= skip && (i + k) < (skip + amount)) {
                ((u8*)data)[i + k - skip] = read[k];
            }
        }
    }

    // the device is going to send us one last interrupt
    driver.waitForInterrupt(_port);

    // advance the lba by wordcount / 1024 (advance once for each 2K block read)
    //_lba += wordcount / 1024;

    driver.release();
    return (wordcount < amount ? wordcount : amount);
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
	if (position == SEEK_SET)
	{
		if (offset > 0)
			_pointer = offset;
	}
	else if (position == SEEK_CUR)
	{
		if (offset < 0 && -offset > position)
			_pointer = 0;
		else
			_pointer += offset;
	}
	else if (position == SEEK_END)
	{
		CPU::panic("NOT IMPLEMENTED");
	}

	return 0;
}

}