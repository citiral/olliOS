//
// Created by Olivier on 24/09/16.
//

#include "ata.h"
#include "atapacketdevice.h"
#include "atapiodevice.h"
#include "../io.h"
#include "../cdefs.h"
#include "../devicemanager.h"
#include <stdio.h>
#include <stdlib.h>

#define PORT_DATA           0x1F0
#define PORT_FEATURE        0x1F1
#define PORT_ERROR          0x1F1
#define PORT_SECTOR_COUNT   0x1F2
#define PORT_SECTOR_NUMER   0x1F3
#define PORT_LBA_LOW        0x1F3
#define PORT_CYLINDER_LOW   0x1F4
#define PORT_LBA_MID        0x1F4
#define PORT_CYLINDER_HIGH  0x1F5
#define PORT_LBA_HIGH       0x1F5
#define PORT_DRIVE          0x1F6
#define PORT_HEAD           0x1F6
#define PORT_COMMAND        0x1F7
#define PORT_STATUS         0x1F7

#define PORT_DEVICE_CONTROL 0x3F6

#define COMMAND_IDENTIFY_DRIVE  0xEC
#define COMMAND_IDENTIFY_PACKET_DRIVE  0xA1

AtaDriver ataDriver = AtaDriver();

AtaDriver::AtaDriver() {
}

void AtaDriver::initialize() {
    // first of all disable irqs and reset both drives
    outb(PORT_DEVICE_CONTROL, (1<<1));

    // detect each device
    unsigned short* data;
    if ((data = (unsigned short*)detectDevice(0)) != nullptr) {
        PRINT_INIT("Found master ATA device: %s", (char*)(data + 27));
    } if ((data = (unsigned short*)detectDevice(1)) != nullptr) {
        PRINT_INIT("Found slave ATA device: %s", (char*)(data + 27));
    }
}

void AtaDriver::selectDevice(int device) {
    // select the given drive
    if (device == 0)
        outb(PORT_DRIVE, 0xA0); // select master
    else
        outb(PORT_DRIVE, 0xB0); // select slave

    // waste a bit of time to make sure the drive select has gone through
    // reading the status register 5 times equals to around 500ns which should be enough (we are expected to wait around 400ns)
    inb(PORT_STATUS);
    inb(PORT_STATUS);
    inb(PORT_STATUS);
    inb(PORT_STATUS);
    inb(PORT_STATUS);
}

unsigned short* AtaDriver::detectDevice(int device) {
    // select the device we want to detect
    selectDevice(device);

    // set these to 0 to properly recognize the signature
    outb(PORT_SECTOR_COUNT, 0);
    outb(PORT_LBA_LOW, 0);
    outb(PORT_LBA_MID, 0);
    outb(PORT_LBA_HIGH, 0);

    // execute the identify command
    outb(PORT_COMMAND, COMMAND_IDENTIFY_DRIVE);

    bool isPacket = false;

    // check if the signature matches that of a packet device
    if (inb(PORT_SECTOR_COUNT) == 0x1 && inb(PORT_LBA_LOW) == 0x1 && inb(PORT_LBA_MID) == 0x14 && inb(PORT_LBA_HIGH) == 0xEB) {
        // do the same again but send the identify packet drive command
        outb(PORT_COMMAND, COMMAND_IDENTIFY_PACKET_DRIVE);
        isPacket = true;
    }

    // if the status register is not 0 there is a device connected
     if (inb(PORT_STATUS) == 0)
         return nullptr;

    // wait until the drive is no longer busy
    waitForBusy();

    // if the drive is about to send an error, fail the detection
    if (waitForDataOrError() == 0) {
        return nullptr;
    }

    // otherwise alloc space for the 512 bytes that are going to be answered
    unsigned short* data = new unsigned short[256];

    // we already know the device is ready to send data so we can just start receiving immediately
    for (int i = 0 ; i < 256 ; i++) {
        // read the 16 bit data from the port
        unsigned short val = inw(PORT_DATA);

        // and convert it to little endian
        data[i] = ((val >> 8) & 0xFF) + ((val & 0xFF) << 8);
    }

    // set 47 to null, this means the device name will be null terminated, and the value at 47 is unimportant (reserved) anyway.
    data[47] = 0;

    // register it to the devicemanager
    if (isPacket) {
        deviceManager.addDevice(new AtaPacketDevice(data));
    } else {
        deviceManager.addDevice(new AtaPioDevice(data));
    }

    return data;
}

void AtaDriver::waitForBusy() {
    // keep checking the status register until the busy bit is false
    char status;
    do {
        status = inb(PORT_STATUS);
    } while ((status & 0x80) == 0x80);
}

bool AtaDriver::waitForDataOrError() {
    // keep checking the status register until the the data bit or error bit is true
    do {
        char status = inb(PORT_STATUS);

        if ((status & 0x08) == 0x08)
            return true;

        if ((status & 0x01) == 0x01)
            return false;

    } while (true);
}