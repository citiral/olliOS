//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_ATAPIODEVICE_H
#define OLLIOS_GIT_ATAPIODEVICE_H

#include "ata/atadevice.h"
#include "cdefs.h"

class AtaPioDevice: public AtaDevice {
public:
    AtaPioDevice(u16 port, unsigned short* data, int device);
    ~AtaPioDevice();

    virtual DeviceType getDeviceType() const;
    virtual void getDeviceInfo(void* deviceinfo) const;

    size_t write(const void* data, size_t amount);
    size_t write(const void* data);
    size_t write(char data);
    size_t read(void* data, size_t amount);
    size_t seek(i32 offset, int position);

private:
	u16 _port;
    unsigned short* _data;
};

#endif //OLLIOS_GIT_ATAPIODEVICE_H
