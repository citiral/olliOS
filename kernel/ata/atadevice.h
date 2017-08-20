#ifndef OLLIOS_ATADEVICE_H
#define OLLIOS_ATADEVICE_H

#include "streams/devices.h"

class AtaPacketDevice: public Device {
public:
    AtaPacketDevice(u16 port, unsigned short* data, u8 drive);
    ~AtaPacketDevice();

    virtual DeviceType getDeviceType() const;
    virtual void getDeviceInfo(void* deviceinfo) const;

    UNIMPLEMENTED(size_t write(const void* data, size_t amount), 0);
    UNIMPLEMENTED(size_t write(const void* data), 0);
    UNIMPLEMENTED(size_t write(char data), 0);
    size_t read(void* data, size_t amount);
    size_t seek(i32 offset, int position);

private:
	u16 _port;
    unsigned short* _data;
    u8 _drive;
    size_t _lba;
};

#endif