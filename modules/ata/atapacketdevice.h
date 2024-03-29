//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_ATAPACKETDEVICE_H
#define OLLIOS_GIT_ATAPACKETDEVICE_H

#include "atadevice.h"
#include "cdefs.h"

namespace ata {
// SCSI command reference
// http://www.seagate.com/staticfiles/support/disc/manuals/Interface%20manuals/100293068c.pdf

class AtaPacketDevice: public AtaDevice {
public:
    AtaPacketDevice(fs::File* ata, AtaChannel* channel, AtaDrive drive, unsigned short* identify_data, u32 deviceId);
    ~AtaPacketDevice();

    virtual size_t write(const void* data, size_t amount, size_t offset);
    virtual size_t read(void* data, size_t amount, size_t offset);

private:
    void waitForBusy();
    void waitForInterrupt();
    bool waitForDataOrError();
};

}

#endif //OLLIOS_GIT_ATAPACKETDEVICE_H
