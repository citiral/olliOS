#ifndef __ISO9660_H
#define __ISO9960_H

#include "fs/filesystem.h"
#include "streams/device.h"
#include "kstd/vector.h"

class Iso9660DirEntry : public DirEntry {
    virtual void advance() {

    }  
};

class Iso9660FileSystem : public FileSystem {
public:
    Iso9660FileSystem(Device* device);
    ~Iso9660FileSystem();
    virtual DirEntry* openDir(const char* path);

private:
    void loadVolumeDescriptors();
    unsigned char* loadPathTable(unsigned char* descriptor);

    Device* _device;
    std::vector<unsigned char*> _descriptors;
    unsigned char* _primarydescriptor;
    unsigned char* _primarypathtable;
};

#endif