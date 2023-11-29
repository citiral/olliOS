#ifndef __ISO9660_H
#define __ISO9960_H

#include "filesystem/file.h"
#include "kstd/string.h"
#include "kstd/vector.h"

class Iso9660File;

class Iso9660FileSystem {
public:
    Iso9660FileSystem(fs::File* dev, size_t offset, size_t length);
    ~Iso9660FileSystem();
    Iso9660File* createRoot();

    u8* readExtend(u32 lba, u32 length);
    void readExtend(u8* buffer, u32 lba, u32 length);
    size_t readRaw(u8* buffer, u32 offset, u32 length);
private:
    void loadVolumeDescriptors();

    size_t _offset;
    size_t _length;
    fs::FileHandle* _dev;
    std::vector<u8*> _descriptors;
    u8* _primarydescriptor;
};

class Iso9660File : public fs::File {
public:
    Iso9660File(Iso9660FileSystem* fs, u8* record, bool skipChildren = false);
    ~Iso9660File();
    fs::FileHandle* open();

    const char* get_name();

    virtual fs::File* create(const char* name, u32 flags);
    virtual fs::File* bind(File* child);

    std::vector<File*> children;
    Iso9660FileSystem* fs;
    u8* record;
    std::string name;
   
private:
    std::string read_name();
    void create_children();
    u8* get_system_use_field(u8 b1, u8 b2);
};

class Iso9660FileHandle : public fs::FileHandle {
public:
    Iso9660FileHandle(Iso9660File* file);

    i32 write(const void* buffer, size_t size, size_t pos);
    i32 read(void* buffer, size_t size, size_t pos);
    size_t get_size();

    fs::File* next_child();
    void reset_child_iterator();

private:
    Iso9660File* _file;
    size_t _child_iter;
};

#endif