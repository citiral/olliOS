#pragma once

#include "singleton.h"
#include "device.h"

#include "streams/device.h"
#include <vector>

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
};

extern DeviceManager deviceManager;