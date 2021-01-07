//
// Created by Olivier on 24/09/16.
//

#ifndef OLLIOS_GIT_ATA_H
#define OLLIOS_GIT_ATA_H

#include "atadevice.h"
#include "threading/semaphore.h"
#include "types.h"
#include "bindings.h"
#include "file.h"

/*#define PORT_DATA           0x1F0
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
#define PORT_STATUS         0x1F7*/

#define PORT_DEFAULT_PRIMARY	0x1F0
#define PORT_DEFAULT_SECONDARY	0x170

#define PORT_DATA			0
#define PORT_FEATURE		1
#define PORT_ERROR			1
#define PORT_SECTOR_COUNT	2
#define PORT_SECTOR_NUMBER	3
#define PORT_LBA_LOW		3
#define PORT_CYLINDER_LOW	4
#define PORT_LBA_MID		4
#define PORT_CYLINDER_HIGH	5
#define PORT_LBA_HIGH		5
#define PORT_DRIVE			6
#define PORT_HEAD			6
#define PORT_COMMAND		7
#define PORT_STATUS			7

#define PORT_DEVICE_CONTROL 0x3F6

#define COMMAND_IDENTIFY_DRIVE  0xEC
#define COMMAND_IDENTIFY_PACKET_DRIVE  0xA1
#define COMMAND_PACKET  0xA0
#define COMMAND_READ  0x20
#define COMMAND_WRITE  0x30
#define COMMAND_FLUSH  0xE7

#define BIT_STATUS_ERR	(1<<0)
#define BIT_STATUS_DRQ	(1<<3)
#define BIT_STATUS_SRV	(1<<4)
#define BIT_STATUS_DF	(1<<5)
#define BIT_STATUS_RDY	(1<<6)
#define BIT_STATUS_BSY	(1<<7)

//TODO: once scheduling is implemented, this should be partially redesigned
// http://www.seagate.com/support/disc/manuals/ata/d1153r17.pdf
// ftp://ftp.seagate.com/acrobat/reference/111-1c.pdf
// IDENTIFY Data structure can be found here on page 91 (129 in PDF)
// http://www.t13.org/documents/uploadeddocuments/docs2006/d1699r3f-ata8-acs.pdf

namespace ata {

enum class AtaDeviceIndex: u8 {
    MASTER = 0,
    SLAVE = 1,
};

void intHandlerAta(u32 interrupt);

// for now we are only going to support one controller
// and use atapi
class AtaDriver {
public:
    AtaDriver();

    // discovers devices and initializes them so after this function they can be used.
	void initialize(fs::File* pci);

	// Don't scan the default addresses. Normally called after a PCI IDE Interface has been detected.
	void disableScanDefaultAddresses();

	// Reset the device;
	void reset(u16 p);

    void printDeviceInformation();

    // if device is 0, it selects the master device, so he will receive all commands. If it is 1, the slave device is selected
    void selectDevice(u16 port, int device);

	// detects a device through the IDENTIFY DRIVE command and // returns a pointer to the returned data if a device has been detected
	// returns the device itself.
	AtaDevice* detectDevice(u16 port, int device);

    // keeps polling the status register until the drive is no longer reporting it is busy
    void waitForBusy(u16 port);

    // keeps polling the status register until the drive is reporting it has data or has an error
    // returns true if there is data available, returns zero if there is an error available
    bool waitForDataOrError(u16 port);

    // keeps waiting until interrupted is set to true. This is then set to false.
    void waitForInterrupt(u16 port);

    void clearInterruptFlag();

    // notifies the ata driver an interrupt has happened
    void notifyInterrupt();

    // grabs or releases the driver, preventing other threads from using it
    void grab();
    void release();

    fs::File* file;

private:
    // This has to be volatile, otherwise codegen might cache it in a register which won't detect changes by interrupt
	volatile bool _interrupted;
	bool _scanDefaultAddresses = true;
	int _lastDevice = -1;
    unsigned int _deviceCount;
    threading::Semaphore _lock;
};

extern AtaDriver driver;

}

#endif //OLLIOS_GIT_ATA_H
