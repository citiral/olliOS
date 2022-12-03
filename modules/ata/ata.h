//
// Created by Olivier on 24/09/16.
//

#ifndef OLLIOS_GIT_ATA_H
#define OLLIOS_GIT_ATA_H

#include "atadevice.h"
#include "atachannel.h"
#include "threading/mutex.h"
#include "types.h"
#include "filesystem/file.h"

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


extern "C" void intHandlerAta(u32 interrupt);

namespace ata {

class m;

enum AtaCommand {
    IdentifyDrive = 0xEC,
    IdentifyPacketDrive = 0xA1,
    Packet = 0xA0,
    Read = 0x20,
    Write = 0x30,
    Flush = 0xE7,
};

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

    // Scans a IDE controller for connected devices
    void scan_ide_controller(u32 primary_data, u32 primary_ctrl, u32 secondary_data, u32 secondary_ctrl);

    // Test if the given drive is present on the ATA channel
    AtaDevice* detect_device(AtaChannel* channel, AtaDrive drive);

    fs::File* file;
    AtaChannel* primary;
    AtaChannel* secondary;

private:
    u32 _foundDeviceCount = 0;
};

extern AtaDriver driver;

}

#endif //OLLIOS_GIT_ATA_H
