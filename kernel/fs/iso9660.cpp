#include "fs/iso9660.h"
#include "streams/device.h"
#include "cdefs.h"
#include "kstd/utility.h"

template <class T>
T readType(u8* descriptor, size_t offset) {
    return *(T*)(descriptor + offset);
}

Iso9660FileSystem::Iso9660FileSystem(Device* device): _device(device) {
    loadVolumeDescriptors();
    //_primarypathtable = loadPathTable(_primarydescriptor);
}

Iso9660FileSystem::~Iso9660FileSystem() {
    for(int i = 0 ; i < _descriptors.size() ; i++) {
        delete[] _descriptors[i];
    }
}

void Iso9660FileSystem::loadVolumeDescriptors() {
    // volume descriptors start at sector 0x10
    if (_device->seek(0x10 * SIZEOF_KB * 2, SEEK_SET) != 0)
        LOG_ERROR("err seeking");

    // keep reading new descriptors
    while (true) {
        u8* descriptor = new u8[2048];
        _device->read(descriptor, 2048);
        
        // untill we encounted the descriptor set terminator
        if (descriptor[0] == 255) {
            delete[] descriptor;
            break;
        }

        if (descriptor[0] == 1) {
            _primarydescriptor = descriptor;
        }

        _descriptors.push_back(descriptor);
        LOG_INFO("Found iso descriptor: %d", descriptor[0]);
    }
}

/*u8* Iso9660FileSystem::loadPathTable(u8* descriptor) {
    u32 pathtablesize = readType<u32>(descriptor, 132);
    u8* pathtablepos = readType<u8*>(descriptor, 140);
    
    LOG_INFO("path table at 0x%X, size is %u", pathtablepos, pathtablesize);
    _device->seek((i32)pathtablepos * SIZEOF_KB * 2, SEEK_SET);

    u8* pathtable = new u8[std::roundup(pathtablesize, 2048u)];
    _device->read(pathtable, std::roundup(pathtablesize, 2048u));

    return pathtable;
}*/

DirEntry* Iso9660FileSystem::openDir(const char* path) {
    return new Iso9660DirEntry();
}

Stream* Iso9660FileSystem::openFile(const char* path) {
    return nullptr;
}

/*DirEntry* Iso9660FileSystem::openDir(const char* path) {
    size_t pathtablesize = readType<size_t>(_primarydescriptor, 132);
    LOG_DEBUG("size is %d", pathtablesize);

    int i = 0;
    while (i < pathtablesize) {
        u8 namelength = _primarypathtable[i];
        printf("%s\n", _primarypathtable+i+8);
        i += 8 + namelength + (namelength%2 == 0 ? 0 : 1);
    }
    /*while (i < pathtablesize) {
        u8 namelength = _primarypathtable[i];
        LOG_DEBUG(" Entry length is %d", namelength);
        i += 9 + namelength;
    }
    
    return nullptr;
}*/

u8* Iso9660FileSystem::readExtend(u32 lba, u32 length) {
    _device->seek(lba * SIZEOF_KB  * 2, SEEK_SET);
    u8* extend = new u8[std::roundup(length, 2048u)];
    _device->read(extend, std::roundup(length, 2048u));
    return extend;
}

Iso9660DirEntry::Iso9660DirEntry() {
    
}

Iso9660DirEntry::~Iso9660DirEntry() {
    
}

bool Iso9660DirEntry::valid() {
    return false;
}

bool Iso9660DirEntry::advance() {
    return false;
}

const char* Iso9660DirEntry::name() {
    return "nope";
}

DirEntryType Iso9660DirEntry::type() {
    return DirEntryType::File;
}
