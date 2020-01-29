//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_ATAPACKETDEVICE_H
#define OLLIOS_GIT_ATAPACKETDEVICE_H

#include "devices/ata/atadevice.h"
#include "cdefs.h"

namespace ata {
// SCSI command reference
// http://www.seagate.com/staticfiles/support/disc/manuals/Interface%20manuals/100293068c.pdf

class AtaPacketDevice: public AtaDevice {
public:
    AtaPacketDevice(u16 port, unsigned short* data, u8 drive);
    ~AtaPacketDevice();

    virtual DeviceType getDeviceType() const;
    virtual void getDeviceInfo(void* deviceinfo) const;

    size_t write(const void* data, size_t amount);
    size_t write(const void* data);
    size_t write(char data);
    size_t read(void* data, size_t amount);
    size_t seek(i32 offset, int position);

private:
    size_t _lba;
};

}

#endif //OLLIOS_GIT_ATAPACKETDEVICE_H
