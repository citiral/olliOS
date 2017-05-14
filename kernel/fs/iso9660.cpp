#include "fs/iso9660.h"
#include "streams/device.h"
#include "cdefs.h"

template <class T>
T readType(unsigned char* descriptor, size_t offset) {
    return *((T*)descriptor[offset]);
}

Iso9660FileSystem::Iso9660FileSystem(Device* device): _device(device) {
    loadVolumeDescriptors();
    _primarypathtable = loadPathTable(_primarydescriptor);
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
        unsigned char* descriptor = new unsigned char[2048];
        _device->read(descriptor, 2048);
        
        // untill we encounted the descriptor set terminator
        if (descriptor[0] == 255) {
            delete[] descriptor;
            break;
        }

        if (descriptor[0] == 255) {
            _primarydescriptor = descriptor;
        }

        _descriptors.push_back(descriptor);
        LOG_INFO("Found iso descriptor: %d", descriptor[0]);
    }
}

unsigned char* Iso9660FileSystem::loadPathTable(unsigned char* descriptor) {
    size_t pathtablesize = readType<size_t>(descriptor, 132);
    unsigned char* pathtablepos = readType<unsigned char*>(descriptor, 140);
    LOG_INFO("path table at 0x%X, size is %d", pathtablepos, pathtablesize);
    _device->seek((i32)pathtablepos, SEEK_SET);

    unsigned char* pathtable = new unsigned char[pathtablesize + pathtablesize%2048];
    _device->read(pathtable, pathtablesize + pathtablesize%2048);

    return pathtable;
}

DirEntry* Iso9660FileSystem::openDir(const char* path) {
    size_t pathtablesize = readType<size_t>(_primarydescriptor, 132);

    int i = 0;
    while (i < pathtablesize) {
        unsigned char namelength = _primarypathtable[i];

        //LOG_INFO("Found dir: %s", _primarypathtable[8]);
        i += 9 + namelength;
    }
    
    return nullptr;
}