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

DirEntry* Iso9660FileSystem::getRoot() {
    u32 lba = readType<u32>(_primarydescriptor, 156 + 2);
    u32 length = readType<u32>(_primarydescriptor, 156 + 10);
    LOG_DEBUG("lba %u, length %u, other %u", lba, length, readType<u16>(_primarydescriptor, 128));
    u8* rootextend = readExtend(lba, length);

    return new Iso9660DirEntry(_device, rootextend, length, 0);
}

u8* Iso9660FileSystem::readExtend(u32 lba, u32 length) {
    _device->seek(lba * SIZEOF_KB  * 2, SEEK_SET);
    u8* extend = new u8[std::roundup(length, 2048u)];
    _device->read(extend, std::roundup(length, 2048u));
    return extend;
}

Iso9660DirEntry::Iso9660DirEntry(Stream* fs, u8* record, u32 length, u32 offset): _fs(fs), _record(record), _length(length), _offset(offset) {

}

Iso9660DirEntry::~Iso9660DirEntry() {
    delete[] _record;
}

bool Iso9660DirEntry::valid() {
    return (_offset < _length) && _record[_offset] > 0;
}

bool Iso9660DirEntry::advance() {
    _offset += readType<u8>(_record, _offset);
    return valid();
}

std::string Iso9660DirEntry::name() {
    // first, we check if there is an alternate name field
    u8* nm = getSystemUseField('N', 'M');

    // if this is null, we return the normal name
    if (nm == nullptr) {
        // but if the filename is 0 or 1, this is respetively the current and parent directory, we translate that to "." and ".."
        // http://alumnus.caltech.edu/~pje/iso9660.html
        if (_record[_offset + 32] == 1 && _record[_offset + 33] == 0) {
            return std::string(".");
        } else if (_record[_offset + 32] == 1 && _record[_offset + 33] == 1) {
            return std::string("..");
        } else {
            return std::string((const char*)_record + _offset + 33, _record[_offset + 32]);
        }
    }

    // otherwise, return the alternate name

    return std::string((const char*)nm + 5, nm[2] - 5);
}

DirEntryType Iso9660DirEntry::type() {
    return (DirEntryType)(readType<u8>(_record, _offset + 25));
}

u8* Iso9660DirEntry::getSystemUseField(u8 b1, u8 b2) {
    // traverse the system use field until we find a compatible field
    u8 length = _record[_offset];
    u8 cur = _record[_offset + 32];
    if (cur % 2 == 0)
        cur++;
    cur += 33;
    while(cur + 1 < length)  {
        if (b1 == _record[_offset + cur] && b2 == _record[_offset + cur + 1])
            return _record + _offset + cur;
        else
            cur += _record[_offset + cur + 2];
    }

    return nullptr;
}

DirEntry* Iso9660DirEntry::openDir() {
    return nullptr;
}

Stream* Iso9660DirEntry::openFile() {
    return nullptr;
}
