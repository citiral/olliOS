//
// Created by Olivier on 24/09/16.
//

#include "threading/thread.h"
#include "ata.h"
#include "atapacketdevice.h"
#include "cpu/interrupt.h"
#include "cpu/io.h"
#include "cdefs.h"
#include "virtualfile.h"
#include "pci/pcidefs.h"
#include <stdio.h>
#include <stdlib.h>

extern "C" void intHandlerAta1_asm();
extern "C" void intHandlerAta2_asm();

namespace ata {

AtaDriver driver = AtaDriver();

extern "C" void intHandlerAta1(u32 interrupt) {
    //printf("ata1\n");
    inb(driver.primary->base + AtaRegister::Status);
    driver.primary->interrupted = true;
    end_interrupt(interrupt);
}

extern "C" void intHandlerAta2(u32 interrupt) {
    //printf("ata2\n");
    inb(driver.secondary->base + AtaRegister::Status);
    driver.secondary->interrupted = true;
    end_interrupt(interrupt);
}

AtaDriver::AtaDriver()
{

}

void AtaDriver::initialize(fs::File* pci)
{
    idt.getEntry(INT_ATA_BUS1).setOffset((u32) &intHandlerAta1_asm);
	idt.getEntry(INT_ATA_BUS2).setOffset((u32) &intHandlerAta2_asm);

    // Create the sys folder for ATA
    file = new fs::VirtualFolder("ata");
    fs::root->get("sys")->bind(file);

    // Scan all PCI devices to see if there is an IDE controller/interface in it
    fs::FileHandle* handle = pci->open();
	fs::File* device;
    while ((device = handle->next_child()) != NULL) {
        u32 dev_class = device->get("class")->read<u8>();
        u32 dev_subclass = device->get("subclass")->read<u8>();
        u32 interface = device->get("interface")->read<u8>();
        u32 pin = device->get("pin")->read<u8>();

        if (dev_class == PCI_CLASS_MASS_STORAGE_CONTROLLER && dev_subclass == PCI_SUBCLASS_IDE_INTERFACE) {
            printf("%s is an IDE mass storage device, interface %X. pin: %X\n", device->get_name(), interface, pin);
            
            u32 primary_base = 0;
            u32 primary_control = 0;
            u32 secondary_base = 0;
            u32 secondary_control = 0;
            u32 bus_master_ide = 0;

            // if bit 0 is set, primary interface is pci native mode (otherwise it is compatibility with default I/O locations)
            if (interface & 1) {
                primary_base = device->get("bars/0")->read<u32>();
                primary_control = device->get("bars/1")->read<u32>();
            } else {
                primary_base = 0x1F0;
                primary_control = 0x3F6;
            }

            // if bit 2 is set, secondary interface is pci native mode (otherwise it is compatibility with default I/O locations)
            if (interface & (1 << 2)) {
                secondary_base = device->get("bars/2")->read<u32>();
                secondary_control = device->get("bars/3")->read<u32>();
            } else {
                secondary_base = 0x170;
                secondary_control = 0x376;
            }

            // if bit 7 is set, it is a bus master ID controller. if clear, it does not support DMA.
            if (interface & (1 << 7)) {
                bus_master_ide = device->get("bars/4")->read<u32>();
            }

            scan_ide_controller(primary_base, primary_control, secondary_base, secondary_control);
        }
    }
}


void AtaDriver::scan_ide_controller(u32 primary_data, u32 primary_ctrl, u32 secondary_data, u32 secondary_ctrl)
{
    printf("scanning %X:%X:%X:%X\n", primary_data, primary_ctrl, secondary_data, secondary_ctrl);
    
    primary = new AtaChannel(primary_data, primary_ctrl, 0);
    secondary = new AtaChannel(secondary_data, secondary_ctrl, 0);

    detect_device(primary, AtaDrive::Master);
    detect_device(primary, AtaDrive::Slave);
    detect_device(secondary, AtaDrive::Master);
    detect_device(secondary, AtaDrive::Slave);
    
}

AtaDevice* AtaDriver::detect_device(AtaChannel* channel, AtaDrive drive)
{
    // set these to 0 to properly recognize the signature
    channel->write_u8(drive, AtaRegister::Seccount, 0);
    channel->write_u8(drive, AtaRegister::Lba0, 0);
    channel->write_u8(drive, AtaRegister::Lba1, 0);
    channel->write_u8(drive, AtaRegister::Lba2, 0);
    for (volatile int i = 0 ; i < 10000000; i++);

    // Disable interrupts on the drive
    channel->write_u8(drive, AtaRegister::Control, 0x02);
    for (volatile int i = 0 ; i < 10000000; i++);

    // execute the identify command
    channel->write_u8(drive, AtaRegister::Command, AtaCommand::IdentifyDrive);
    for (volatile int i = 0 ; i < 10000000; i++);
	
	// Check the status register
	u8 status = channel->read_u8(drive, AtaRegister::Status);

	// If it is 0 there is no drive here.
	// BIT_STATUS_DF will be high if there is a fault with the drive (if I read the specs correctly)
	if (status == 0 || (status & BIT_STATUS_DF)) {
        //printf("Drive returned error.\n");
		return nullptr;
    }

    // Now we wait for the BSY bit of the drive to clear
    while ((status & BIT_STATUS_BSY)) {
        status = channel->read_u8(drive, AtaRegister::Status);
    }

    u8 seccount = channel->read_u8(drive, AtaRegister::Seccount);
    u8 lba0 = channel->read_u8(drive, AtaRegister::Lba0);
    u8 lba1 = channel->read_u8(drive, AtaRegister::Lba1);
    u8 lba2 = channel->read_u8(drive, AtaRegister::Lba2);

    if (seccount == 1 && lba0 == 1 && lba1 == 0x14 && lba2 == 0xEB) {
        printf("Device is packet device\n");
    
        // Packet devices require another Identifyt packet drive command
        channel->write_u8(drive, AtaRegister::Command, AtaCommand::IdentifyPacketDrive);
        for (volatile int i = 0 ; i < 10000000; i++);

        // we alloc space for the 512 bytes that are going to be answered
        unsigned short* data = new unsigned short[256];

        // we already know the device is ready to send data so we can just start receiving immediately
        for (int i = 0 ; i < 256 ; i++) {
            // read the 16 bit data from the port
            u16 val = channel->read_u16(drive, AtaRegister::Data);

            // and convert it to little endian
            data[i] = ((val >> 8) & 0xFF) + ((val & 0xFF) << 8);

            printf("%c%c", data[i], data[i] >> 8);
        }

        // set 47 to null, this means the device name will be null terminated, and the value at 47 is unimportant (reserved) anyway.
        data[47] = 0;

        // register it to the devicemanager
        printf("Found device!\n");
        return new AtaPacketDevice(file, channel, drive, data, _foundDeviceCount++);

    } else if (seccount == 1 && lba0 == 1 && lba1 == 0 && lba2 == 0) {
        printf("Device is pio device.. however unimplemented for now.\n");
        return nullptr;
    } else {
        printf("Unknown device signature: 0x%X 0x%X 0x%X 0x%X\n", seccount, lba0, lba1, lba2);
        return nullptr;
    }
}

}
