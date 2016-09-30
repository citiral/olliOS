//
// Created by Olivier on 30/09/16.
//

#include "atapacketdevice.h"

AtaPacketDevice::AtaPacketDevice(unsigned short* data): _data(data) {
}

AtaPacketDevice::~AtaPacketDevice() {
    delete[] _data;
}

DeviceType AtaPacketDevice::getDeviceType() const {
    return DeviceType::Storage;
}

const char* AtaPacketDevice::getDeviceName() const {
    return (char*)(_data + 27);
}