#pragma once

#include "util/singleton.h"
#include "devices/device.h"
#include "kstd/vector.h"

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();

    void addDevice(Device* device);
    void destroyDevice(Device* device);

    Device* getDevice(DeviceType type, size_t device);
    std::vector<Device*>& getDevices(DeviceType type);

private:
    std::vector<Device*> keyboardDevices;
    std::vector<Device*> screenDevices;
	std::vector<Device*> storageDevices;
	std::vector<Device*> pciDevices;
	std::vector<Device*> serialDevices;
};

extern DeviceManager deviceManager;