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

private:
    std::vector<Device*> _devices;
};

extern DeviceManager deviceManager;