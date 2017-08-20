#include "devicemanager.h"
#include "cpu.h"

DeviceManager deviceManager;

DeviceManager::DeviceManager() {

}

DeviceManager::~DeviceManager() {
    // delete all remaining devices
    for (size_t i = 0 ; i < keyboardDevices.size(); i++)
        delete keyboardDevices[i];
    for (size_t i = 0 ; i < screenDevices.size(); i++)
        delete screenDevices[i];
    for (size_t i = 0 ; i < storageDevices.size(); i++)
		delete storageDevices[i];
	for (size_t i = 0 ; i < pciDevices.size(); i++)
		delete pciDevices[i];
	for (size_t i = 0 ; i < pciDevices.size(); i++)
        delete serialDevices[i];
}

void DeviceManager::addDevice(Device* device) {
    getDevices(device->getDeviceType()).push_back(device);
}

void DeviceManager::destroyDevice(Device* device) {
    size_t index = getDevices(device->getDeviceType()).find(device);

    if (index != (size_t) -1)
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
		case DeviceType::PCI:
			return pciDevices;
			break;
		case DeviceType::Serial:
			return serialDevices;
			break;
        default:
			// TODO: panic since this could never really happen
			CPU::panic("Attempt to get unknown device type");
            return keyboardDevices;
            break;
    }
}