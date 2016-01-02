#pragma once
#include "singleton.h"
#include "rtti.h"

enum class DeviceType {
    Keyboard,
    Screen,
};

//there can ever be only 1 device manager
class DeviceManager : public Singleton<DeviceManager>
{
private:
  DeviceManager();
  ~DeviceManager();

public:
    void registerDevice(DeviceType type);

};
