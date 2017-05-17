#ifndef __ISO9660_H
#define __ISO9960_H

#include "fs/filesystem.h"
#include "streams/device.h"
#include "kstd/vector.h"

class Iso9660DirEntry : public DirEntry {
public:
    Iso9660DirEntry();
    ~Iso9660DirEntry();
    virtual void advance();
    virtual void open();

private:
    // the record is the header containing file/dir information
    u8* record;

    // The extend is the actual data of the file
    u8* extend;
};

class Iso9660FileSystem : public FileSystem {
public:
    Iso9660FileSystem(Device* device);
    ~Iso9660FileSystem();
    virtual DirEntry* openDir(const char* path);

private:
    void loadVolumeDescriptors();
    u8* loadPathTable(u8* descriptor);

    Device* _device;
    std::vector<u8*> _descriptors;
    u8* _primarydescriptor;
    u8* _primarypathtable;
};

#endif