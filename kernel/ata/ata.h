//
// Created by Olivier on 24/09/16.
//

#ifndef OLLIOS_GIT_ATA_H
#define OLLIOS_GIT_ATA_H

#include "types.h"

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
#define COMMAND_PACKET      0xA0

//TODO: once scheduling is implemented, this should be partially redesigned
// http://www.seagate.com/support/disc/manuals/ata/d1153r17.pdf
// ftp://ftp.seagate.com/acrobat/reference/111-1c.pdf
// http://www.t13.org/documents/uploadeddocuments/docs2006/d1699r3f-ata8-acs.pdf

enum class AtaDeviceIndex: u8 {
    MASTER = 0,
    SLAVE = 1,
};

class AtaDevice {
public:
private:
    unsigned short* identify;
    AtaDeviceIndex index;
};

// for now we are only going to support one controller
// and use atapi
class AtaDriver {
public:
    AtaDriver();

    // discovers devices and initializes them so after this function they can be used.
    void initialize();

    void printDeviceInformation();

    // if device is 0, it selects the master device, so he will receive all commands. If it is 1, the slave device is selected
    void selectDevice(int device);

    // detects a device through the IDENTIFY DRIVE command and returns a pointer to the returned data if a device has been detected
    unsigned short* detectDevice(int device);

    // keeps polling the status register until the drive is no longer reporting it is busy
    void waitForBusy();

    // keeps polling the status register until the drive is reporting it has data or has an error
    // returns true if there is data available, returns zero if there is an error available
    bool waitForDataOrError();

    // keeps waiting until interrupted is set to true. This is then set to false.
    void waitForInterrupt();

    // notifies the ata driver an interrupt has happened
    void notifyInterrupt();

private:
    // This has to be volatile, otherwise codegen might cache it in a register which won't detect changes by interrupt
    volatile bool _interrupted;
};



extern AtaDriver ataDriver;

#endif //OLLIOS_GIT_ATA_H
