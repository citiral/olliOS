//
// Created by Olivier on 24/09/16.
//

#ifndef OLLIOS_GIT_ATA_H
#define OLLIOS_GIT_ATA_H

#include <types.h>

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

private:
    // if device is 0, it selects the master device, so he will receive all commands. If it is 1, the slave device is selected
    void selectDevice(int device);

    // detects a device through the IDENTIFY DRIVE command and returns a pointer to the returned data if a device has been detected
    unsigned short* detectDevice(int device);

    // keeps polling the status register until the drive is no longer reporting it is busy
    void waitForBusy();

    // keeps polling the status register until the drive is reporting it has data or has an error
    // returns true if there is data available, returns zero if there is an error available
    bool waitForDataOrError();
};



extern AtaDriver ataDriver;

#endif //OLLIOS_GIT_ATA_H
