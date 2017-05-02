#ifndef __ISO9660_H
#define __ISO9960_H

#include "fs/filesystem.h"
#include "streams/device.h"

class Iso9660DirEntry : public DirEntry {
    
};

class Iso9660FileSystem : public FileSystem {
public:
    Iso9660FileSystem(Device* device);
    ~Iso9660FileSystem();

    virtual DirEntry* openDir(const char* path);

private:
    void LoadVolumeDescriptors();

    Device* _device;
    unsigned char* _descriptor;     
};

#endif