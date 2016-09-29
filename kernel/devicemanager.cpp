#include "devicemanager.h"

DeviceManager deviceManager;

DeviceManager::DeviceManager() {

}

DeviceManager::~DeviceManager() {
    // delete all remaining devices
    for (int i = 0 ; i < keyboardDevices.size(); i++)
        delete keyboardDevices[i];
    for (int i = 0 ; i < screenDevices.size(); i++)
        delete screenDevices[i];
    for (int i = 0 ; i < storageDevices.size(); i++)
        delete storageDevices[i];
}

void DeviceManager::addDevice(Device* device) {
    getDevices(device->getDeviceType()).push_back(device);
}

void DeviceManager::destroyDevice(Device* device) {
    int index = getDevices(device->getDeviceType()).find(device);

    if (index != -1)
        getDevices(device->getDeviceType()).erase(index);
}

Device* DeviceManager::getDevice(DeviceType type, size_t device) {
    if (device < getDevices(type).size())
        return getDevices(type)[device];
    else
        return nullptr;
}

std::vector<Device*>& DeviceManager::getDevices(DeviceType type) {
    switch (type) {
        case DeviceType::Keyboard:
            return keyboardDevices;
            break;
        case DeviceType::Screen:
            return screenDevices;
            break;
        case DeviceType::Storage:
            return storageDevices;
            break;
        default:
            // TODO: panic since this could never really happen
            return keyboardDevices;
            break;
    }
}