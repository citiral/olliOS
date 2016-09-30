//
// Created by Olivier on 30/09/16.
//

#include "atapiodevice.h"

AtaPioDevice::AtaPioDevice(unsigned short* data): _data(data) {
}

AtaPioDevice::~AtaPioDevice() {
    delete[] _data;
}

DeviceType AtaPioDevice::getDeviceType() const {
    return DeviceType::Storage;
}

const char* AtaPioDevice::getDeviceName() const {
    return (char*)(_data + 27);
}