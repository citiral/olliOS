//
// Created by Olivier on 30/09/16.
//

#include "ata/atapiodevice.h"

AtaPioDevice::AtaPioDevice(unsigned short* data): _data(data) {
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
