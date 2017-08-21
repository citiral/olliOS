#ifndef OLLIOS_RAMFS_H
#define OLLIOS_RAMFS_H

#include "fs/filesystem.h"
#include "devices/blockdevice.h"
#include "kstd/vector.h"

class RamFileSystem;

class RamDirEntry : public DirEntry {
public:
    RamDirEntry(BlockDevice* fs, u8* record, u32 length, u32 offset);
    ~RamDirEntry();

    virtual bool valid();
    virtual bool advance();
    virtual std::string name();
    virtual DirEntryType type();
    virtual DirEntry* openDir();
    virtual BlockDevice* openFile();

private:
    std::vector<DirEntry*> _contents;
};

class RamFileSystem : public FileSystem {
public:
    RamFileSystem(BlockDevice* device);
    ~RamFileSystem();
    
    DirEntry* getRoot();
};

#endif