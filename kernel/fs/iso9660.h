#ifndef __ISO9660_H
#define __ISO9960_H

#include "fs/filesystem.h"
#include "streams/device.h"
#include "kstd/vector.h"

class Iso9660DirEntry : public DirEntry {
public:
    Iso9660DirEntry();
    ~Iso9660DirEntry();
    virtual bool valid();
    virtual bool advance();
    virtual const char* name();
    virtual DirEntryType type();

private:
    u8* recordlist;
    u8* recordoffset;
};

class Iso9660FileSystem : public FileSystem {
public:
    Iso9660FileSystem(Device* device);
    ~Iso9660FileSystem();
    
    virtual DirEntry* openDir(const char* path);
    virtual Stream* openFile(const char* path);

private:
    void loadVolumeDescriptors();
    u8* readExtend(u32 lba, u32 length);

    Device* _device;
    std::vector<u8*> _descriptors;
    u8* _primarydescriptor;
};

#endif