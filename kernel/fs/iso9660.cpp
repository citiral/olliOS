#include "fs/iso9660.h"
#include "streams/device.h"
#include "cdefs.h"

Iso9660FileSystem::Iso9660FileSystem(Device* device): _device(device) {
    if (device->seek(0x10 * SIZEOF_KB * 2, SEEK_SET) != 0)
        LOG_ERROR("err seeking");
    
    _descriptor = new unsigned char[2048];
    device->read(_descriptor, 2048);
    
    LOG_INFO("iso fs id is: %d", _descriptor[0]);
}

Iso9660FileSystem::~Iso9660FileSystem() {
    delete[] _descriptor;
}

DirEntry* Iso9660FileSystem::openDir(const char* path) {
    return nullptr;
}