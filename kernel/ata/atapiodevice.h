//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_ATAPIODEVICE_H
#define OLLIOS_GIT_ATAPIODEVICE_H

#include "streams/device.h"
#include "cdefs.h"

class AtaPioDevice: public Device {
public:
    AtaPioDevice(unsigned short* data);
    ~AtaPioDevice();

    virtual DeviceType getDeviceType() const;
    virtual const char* getDeviceName() const;

    UNIMPLEMENTED(size_t write(const void* data, size_t amount), 0);
    UNIMPLEMENTED(size_t write(const void* data), 0);
    UNIMPLEMENTED(size_t write(char data), 0);
    UNIMPLEMENTED(size_t read(void* data, size_t amount), 0);
    UNIMPLEMENTED(size_t seek(i32 offset, int position), 0);

private:
    unsigned short* _data;
};

#endif //OLLIOS_GIT_ATAPIODEVICE_H
