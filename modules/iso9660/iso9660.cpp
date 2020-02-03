#include "iso9660.h"
#include "kstd/utility.h"
#include "cdefs.h"
#include "cpu.h"

#define DIRECTORY_RECORD_HIDDEN 0x1
#define DIRECTORY_RECORD_IS_DIRECTORY 0x2
#define DIRECTORY_RECORD_IS_ASSOCIATED_FILE 0x4
#define DIRECTORY_RECORD_HAS_EXTENDED_ATTRIBUTES 0x8
#define DIRECTORY_RECORD_NOT_FINAL 0xF0

using namespace bindings;

template <class T>
T readType(u8* descriptor, size_t offset) {
    return *(T*)(descriptor + offset);
}

Iso9660FileSystem::Iso9660FileSystem(bindings::Binding* dev): _bind(dev) {
    loadVolumeDescriptors();
    //_primarypathtable = loadPathTable(_primarydescriptor);
}

Iso9660FileSystem::~Iso9660FileSystem() {
    for(size_t i = 0 ; i < _descriptors.size() ; i++) {
        delete[] _descriptors[i];
    }
}

void Iso9660FileSystem::loadVolumeDescriptors() {
    // keep reading new descriptors
    size_t offset = 0x10 * SIZEOF_KB * 2;

    while (true) {
        u8* descriptor = new u8[2048];
        _bind->read(descriptor, 2048, offset);
        offset += 2048;
        
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

    Iso9660Binding* root = createRoot();
    root->name = "root";
    _bind->add(root);
}

Iso9660Binding* Iso9660FileSystem::createRoot() {
    u32 lba = readType<u32>(_primarydescriptor, 156 + 2);
    u32 length = readType<u32>(_primarydescriptor, 156 + 10);
    u8* rootextend = readExtend(lba, length);

    return new Iso9660Binding(this, rootextend);
}

u8* Iso9660FileSystem::readExtend(u32 lba, u32 length) {
    u8* extend = new u8[length];
    _bind->read(extend, length, lba * SIZEOF_KB  * 2);
    return extend;
}

void Iso9660FileSystem::readExtend(u8* buffer, u32 lba, u32 length) {
    _bind->read(buffer, length, lba * SIZEOF_KB  * 2);
}

size_t Iso9660FileSystem::readRaw(u8* buffer, u32 offset, u32 length) {
    return _bind->read(buffer, length, offset);
}

Iso9660Binding::Iso9660Binding(Iso9660FileSystem* fs, u8* record): _fs(fs), _record(record), OwnedBinding("") {
    name = get_name();

    on_read([](OwnedBinding* binding, void* buffer, size_t size, size_t offset) {
        Iso9660Binding* t = (Iso9660Binding*)binding;
        return t->read(buffer, size, offset);
    });

    if (_record[25] & DIRECTORY_RECORD_IS_DIRECTORY) {
        create_children();
    }
}

std::string Iso9660Binding::get_name()
{
    // first, we check if there is an alternate name field
    u8* nm = get_system_use_field('N', 'M');

    // if this is null, we return the normal name
    if (nm == nullptr) {
        // but if the filename is 0 or 1, this is respectively the current and parent directory, we translate that to "." and ".."
        // http://alumnus.caltech.edu/~pje/iso9660.html
        if (_record[32] == 1 && _record[33] == 0) {
            return std::string(".");
        } else if (_record[32] == 1 && _record[33] == 1) {
            return std::string("..");
        } else {
            //printf("file name length: %d\n", (u32) _record[32]);
            return std::string((const char*)_record + 33, _record[32]);
        }
    }

    // otherwise, return the alternate name
    return std::string((const char*)nm + 5, nm[2] - 5);
}

void Iso9660Binding::create_children()
{
    // Read the position and length of the directory record
    u32 lba = readType<u32>(_record, 2);
    u32 length = readType<u32>(_record, 10);
    
    // Read the actual directory data
    u8* extend = _fs->readExtend(lba, length);

    // Keep parsing children until we went through all data
    size_t offset = 0;


    while (offset < length) {

        // Skip directories . and ..
        if (extend[32] == 1 && extend[33] == 0) {
            
        } else if (extend[32] == 1 && extend[33] == 1) {
        } else {
            add(new Iso9660Binding(_fs, extend));
        }
        
        offset += extend[0];
        extend += extend[0];

        while (offset < length && extend[0] == 0) {
            offset++;
            extend++;
        } 
    }
}

size_t Iso9660Binding::read(void* buffer, size_t size, size_t offset)
{
    u32 lba = readType<u32>(_record, 2);
    u32 length = readType<u32>(_record, 10);

    if (offset >= length) {
        return 0;
    } else if (offset + size >= length) {
        size = length - offset;
    }

    return _fs->readRaw((u8*) buffer, lba * SIZEOF_KB * 2 + offset, size);
}

u8* Iso9660Binding::get_system_use_field(u8 b1, u8 b2) {
    // traverse the system use field until we find a compatible field
    u8 length = _record[0];
    u8 cur = _record[32];
    if (cur % 2 == 0)
        cur++;
    cur += 33;
    while(cur + 1 < length)  {
        if (b1 == _record[cur] && b2 == _record[cur + 1])
            return _record + cur;
        else
            cur += _record[cur + 2];
    }

    return nullptr;
}

/*
Iso9660DirEntry::Iso9660DirEntry(BlockDevice* fs, u8* record, u32 length, u32 offset): _fs(fs), _record(record), _length(length), _offset(offset) {

}

Iso9660DirEntry::~Iso9660DirEntry() {
    delete[] _record;
}

bool Iso9660DirEntry::valid() {
    return (_offset < _length);
}

bool Iso9660DirEntry::advance() {
    // first, move over this file
    _offset += readType<u8>(_record, _offset);

    // then, because there can be 0 padding after files, keep skipping 0's, until we find something or reach the end of the directory
    // this is ugly, but i'm not sure there is another way.

    
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
    u8 flag = readType<u8>(_record, _offset + 25);
    if ((flag&2) == 0)
        return DirEntryType::File;
    else 
        return DirEntryType::Folder;
}

DirEntry* Iso9660DirEntry::openDir() {
    u32 lba = readType<u32>(_record, _offset + 2);
    u32 length = readType<u32>(_record, _offset + 10);

    _fs->seek(lba * SIZEOF_KB  * 2, SEEK_SET);
    u8* extend = new u8[std::roundup(length, 2048u)];
    _fs->read(extend, std::roundup(length, 2048u));

    return new Iso9660DirEntry(_fs, extend, length, 0);
}

BlockDevice* Iso9660DirEntry::openFile() {
    u32 lba = readType<u32>(_record, _offset + 2);
    u32 length = readType<u32>(_record, _offset + 10);

    _fs->seek(lba * SIZEOF_KB  * 2, SEEK_SET);
    u8* data = new u8[std::roundup(length, 2048u)];
    _fs->read(data, std::roundup(length, 2048u));

    return new MemoryStream(data, length);
}

DirEntry* Iso9660DirEntry::createDir(std::string name) {
    UNUSED(name);
	CPU::panic("Directory creation not supported on ISO9660 implementation");
	return nullptr;
}*/