//
// Created by Olivier on 30/09/16.
//

#include "ata/atapiodevice.h"
#include "cpu.h"

AtaPioDevice::AtaPioDevice(u16 port, unsigned short* data, int device): AtaDevice(port, data, device) {
	printf("ATA PIO Device created!\n");
}

AtaPioDevice::~AtaPioDevice() {
    delete[] _data;
}

DeviceType AtaPioDevice::getDeviceType() const {
    return DeviceType::Storage;
}

void AtaPioDevice::getDeviceInfo(void* deviceinfo) const
{
	DeviceStorageInfo* info = (DeviceStorageInfo*)deviceinfo;
	info->deviceInfo.name = (char*)(_data + 27);
}

size_t AtaPioDevice::write(const void* data, size_t amount)
{
	UNUSED(data);
	UNUSED(amount);
	CPU::panic("Write not implemented");
	return 0;
}

size_t AtaPioDevice::write(const void* data)
{
	UNUSED(data);
	CPU::panic("Write not implemented");
	return 0;
}

size_t AtaPioDevice::write(char data)
{
	UNUSED(data);
	CPU::panic("Write not implemented");
	return 0;
}

size_t AtaPioDevice::read(void* data, size_t amount)
{
	UNUSED(data);
	UNUSED(amount);
	CPU::panic("Read not implemented");
	return 0;
}

size_t AtaPioDevice::seek(i32 offset, int position)
{
	UNUSED(offset);
	UNUSED(position);
	CPU::panic("Seek not implemented");
	return 0;
}