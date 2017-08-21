#ifndef __ISO9660_H
#define __ISO9960_H

#include "fs/filesystem.h"
#include "devices/blockdevice.h"
#include "kstd/vector.h"

class Iso9660FileSystem;

class Iso9660DirEntry : public DirEntry {
public:
    Iso9660DirEntry(BlockDevice* fs, u8* record, u32 length, u32 offset);
    ~Iso9660DirEntry();

    virtual bool valid();
    virtual bool advance();
    virtual std::string name();
    virtual DirEntryType type();
    virtual DirEntry* openDir();
    virtual BlockDevice* openFile();
	virtual DirEntry* createDir(std::string name);

    u8* getSystemUseField(u8 b1, u8 b2);

private:
	BlockDevice* _fs;
    u8* _record;
    u32 _length;
    u32 _offset;
};

class Iso9660FileSystem : public FileSystem {
public:
    Iso9660FileSystem(BlockDevice* device);
    ~Iso9660FileSystem();
    
    DirEntry* getRoot();

private:
    void loadVolumeDescriptors();
    u8* readExtend(u32 lba, u32 length);

    BlockDevice* _device;
    std::vector<u8*> _descriptors;
    u8* _primarydescriptor;
};

#endif