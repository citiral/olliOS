#ifndef __DEVICE_H
#define __DEVICE_H

#include "streams/stream.h"

enum class DeviceType: u8 {
    Keyboard = 0,
    Screen = 1,
    Storage = 2,
};

/**
* A device represents both a hardware device (Keyboard, screen,..) and their
* respective driver. The interface exposed in the base Device class
* represents the way anything should interact with a driver, and as a result
* a device. A device needs to be registered with the devicemanager. A device
* has the same interface as a file.
*/
class Device : public Stream {
public:
    virtual ~Device();

    //returns the type of the device
    virtual DeviceType getDeviceType() const = 0;
    //returns the name of the device, this need not be unique.
    virtual const char* getDeviceName() const = 0;
};

#endif /* end of include guard: __DEVICE_H */
