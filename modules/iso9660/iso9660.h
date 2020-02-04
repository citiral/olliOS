#ifndef __ISO9660_H
#define __ISO9960_H

#include "bindings.h"
#include "kstd/vector.h"

class Iso9660Binding;

class Iso9660FileSystem {
public:
    Iso9660FileSystem(bindings::Binding* dev);
    ~Iso9660FileSystem();
    Iso9660Binding* createRoot();

    u8* readExtend(u32 lba, u32 length);
    void readExtend(u8* buffer, u32 lba, u32 length);
    size_t readRaw(u8* buffer, u32 offset, u32 length);
private:
    void loadVolumeDescriptors();

    bindings::Binding* _bind;
    std::vector<u8*> _descriptors;
    u8* _primarydescriptor;
};

class Iso9660Binding : public bindings::OwnedBinding {
public:
    Iso9660Binding(Iso9660FileSystem* fs, u8* record);

private:
    size_t read(void* buffer, size_t size, size_t offset);

    std::string get_name();
    void create_children();
    u8* get_system_use_field(u8 b1, u8 b2);

    Iso9660FileSystem* _fs;
    u8* _record;
};

/*class Iso9660FileSystem;

class Iso9660DirEntry {
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

class Iso9660FileSystem {
public:
    Iso9660FileSystem(BlockDevice* device);
    ~Iso9660FileSystem();
    
    DirEntry* getRoot();

};*/

#endif