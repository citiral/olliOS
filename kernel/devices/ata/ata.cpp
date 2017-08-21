//
// Created by Olivier on 24/09/16.
//

#include "devices/ata/ata.h"
#include "devices/ata/atapacketdevice.h"
#include "devices/ata/atapiodevice.h"
#include "io.h"
#include "cdefs.h"
#include "devicemanager.h"
#include <stdio.h>
#include <stdlib.h>

AtaDriver ataDriver = AtaDriver();

AtaDriver::AtaDriver(): _interrupted(false) {
}

void AtaDriver::initialize() {
	_interrupted = false;
	
	if (_scanDefaultAddresses)
	{
		// detect each device
		AtaDevice* device;
		if ((device = detectDevice(PORT_DEFAULT_PRIMARY, 0)) != nullptr) {
			LOG_INFO("Found primary master ATA device: '%s'", device->getName().c_str());
		}
		if ((device = detectDevice(PORT_DEFAULT_PRIMARY, 1)) != nullptr) {
			LOG_INFO("Found primary slave ATA device: '%s'", device->getName().c_str());
		}
		if ((device = detectDevice(PORT_DEFAULT_SECONDARY, 0)) != nullptr) {
			LOG_INFO("Found secondary master ATA device: '%s'", device->getName().c_str());
		}
		if ((device = detectDevice(PORT_DEFAULT_SECONDARY, 1)) != nullptr) {
			LOG_INFO("Found secondary slave ATA device: '%s'", device->getName().c_str());
		}
	}
}

void AtaDriver::disableScanDefaultAddresses()
{
	_scanDefaultAddresses = false;
}

void AtaDriver::reset(u16 p)
{
	outb(p, 0b00000100);

	// Just a very simple spinloop
	for (int i = 0; i < 1000; i++)
		asm volatile ("pause");

	outb(p, 0b00000000);
}

void AtaDriver::selectDevice(u16 p, int device) {
	// We only need to do this if we actually select a different device.
	if (device != _lastDevice)
	{
		// select the given drive
		if (device == 0)
			outb(p+PORT_DRIVE, 0xA0); // select master
		else
			outb(p+PORT_DRIVE, 0xB0); // select slave

		// waste a bit of time to make sure the drive select has gone through
		// reading the status register 5 times equals to around 500ns which should be enough (we are expected to wait around 400ns)
		inb(p+PORT_STATUS);
		inb(p+PORT_STATUS);
		inb(p+PORT_STATUS);
		inb(p+PORT_STATUS);
		inb(p+PORT_STATUS);


		_lastDevice = device;
	}
}

AtaDevice* AtaDriver::detectDevice(u16 p, int device) {
    // select the device we want to detect
	selectDevice(p, device);

    // set these to 0 to properly recognize the signature
    outb(p+PORT_SECTOR_COUNT, 0);
    outb(p+PORT_LBA_LOW, 0);
    outb(p+PORT_LBA_MID, 0);
    outb(p+PORT_LBA_HIGH, 0);

    // execute the identify command
	outb(p+PORT_COMMAND, COMMAND_IDENTIFY_DRIVE);
	
	// Check the status register
	// It will be 0 if no drive exists
	// BIT_STATUS_DF will be high if there is a fault with the drive (if I read the specs correctly)
	u8 status = inb(p+PORT_STATUS);
	if (status == 0 || (status & BIT_STATUS_DF) > 0)
		return nullptr;

    bool isPacket = false;

    // check if the signature matches that of a packet device
    if (inb(p+PORT_SECTOR_COUNT) == 0x1 && inb(p+PORT_LBA_LOW) == 0x1 && inb(p+PORT_LBA_MID) == 0x14 && inb(p+PORT_LBA_HIGH) == 0xEB) {
        // do the same again but send the identify packet drive command
        outb(p+PORT_COMMAND, COMMAND_IDENTIFY_PACKET_DRIVE);
        isPacket = true;
    }

    // wait until the drive is ready
    waitForInterrupt(p);
    /*waitForBusy();

    // if the drive is about to send an error, fail the detection
    if (waitForDataOrError() == 0) {
        return nullptr;
    }*/

    // otherwise alloc space for the 512 bytes that are going to be answered
    unsigned short* data = new unsigned short[256];

    // we already know the device is ready to send data so we can just start receiving immediately
    for (int i = 0 ; i < 256 ; i++) {
        // read the 16 bit data from the port
        unsigned short val = inw(p+PORT_DATA);

        // and convert it to little endian
        data[i] = ((val >> 8) & 0xFF) + ((val & 0xFF) << 8);
    }

    // set 47 to null, this means the device name will be null terminated, and the value at 47 is unimportant (reserved) anyway.
    data[47] = 0;

	// register it to the devicemanager
	AtaDevice* atadevice;
    if (isPacket) {
		atadevice = new AtaPacketDevice(p, data, device);
        deviceManager.addDevice(atadevice);
    } else {
		atadevice = new AtaPioDevice(p, data, device);
        deviceManager.addDevice(atadevice);
    }

    return atadevice;
}

void AtaDriver::waitForBusy(u16 p) {
    // keep checking the status register until the busy bit is false
    char status;
    do {
        status = inb(p+PORT_STATUS);
        asm volatile ("pause");
    } while ((status & 0x80) == 0x80);
}

bool AtaDriver::waitForDataOrError(u16 p) {
    // keep checking the status register until the the data bit or error bit is true
    do {
        char status = inb(p+PORT_STATUS);

        if ((status & 0x08) == 0x08)
            return true;

        if ((status & 0x01) == 0x01)
            return false;
        asm volatile ("pause");
    } while (true);
}

void AtaDriver::waitForInterrupt(u16 p) {
    while (_interrupted == false) {
        asm volatile("pause");        
    }
    _interrupted = false;

    // read the status register to acknowledge the IRQ
    inb(p+PORT_STATUS);
}

void AtaDriver::notifyInterrupt() {
    _interrupted = true;
}