#pragma once

#include "singleton.h"
#include "device.h"

//there can ever be only 1 device manager
class DeviceManager : public Singleton<DeviceManager>
{
private:
  DeviceManager();
  ~DeviceManager();

public:
    void registerDevice(DeviceType type);

};
