#include "virtualfile.h"
#include "cdefs.h"
#include "threading/thread.h"

using namespace fs;

VirtualFileHandle::VirtualFileHandle(VirtualFile* file): _file(file)
{

}

i32 VirtualFileHandle::write(const void* buffer, size_t size, size_t pos)
{
    size_t chunk = pos / VIRTUAL_FILE_CHUNK_SIZE;
    size_t chunk_offset = pos % VIRTUAL_FILE_CHUNK_SIZE;
    size_t chunk_remaining = VIRTUAL_FILE_CHUNK_SIZE - chunk_offset;

    i32 written = 0;

    while (size > 0) {
        if (chunk == _file->data.size()) {
            _file->data.push_back(new uint8_t[VIRTUAL_FILE_CHUNK_SIZE]);
        }

        size_t length = size < chunk_remaining ? size : chunk_remaining;
        memcpy(_file->data[chunk] + chunk_offset, buffer, length);

        chunk++;
        chunk_offset = 0;
        chunk_remaining = VIRTUAL_FILE_CHUNK_SIZE;
        size -= length;

        buffer = ((uint8_t*)buffer) + length;
        written += length;
    }

    _file->size += written;

    return written;
}

i32 VirtualFileHandle::read(void* buffer, size_t size, size_t pos)
{
    if (pos >= _file->size) {
        return 0;
    } else if (pos + size >= _file->size) {
        size = _file->size - pos;
    }

    i32 read = 0;

    size_t chunk = pos / VIRTUAL_FILE_CHUNK_SIZE;
    size_t chunk_offset = pos % VIRTUAL_FILE_CHUNK_SIZE;
    size_t chunk_remaining = VIRTUAL_FILE_CHUNK_SIZE - chunk_offset;

    while (size > 0) {
        size_t length = size < chunk_remaining ? size : chunk_remaining;
        memcpy(buffer, _file->data[chunk] + chunk_offset, length);
        chunk++;
        chunk_offset = 0;
        chunk_remaining = VIRTUAL_FILE_CHUNK_SIZE;
        size -= length;

        buffer = ((uint8_t*)buffer) + length;
        read += length;
    }

    return read;
}

size_t VirtualFileHandle::get_size()
{
    return _file->size;
}

File* VirtualFileHandle::next_child()
{
    return NULL;
}

void VirtualFileHandle::reset_child_iterator()
{
}




VirtualFile::VirtualFile(std::string name): name(name), data(), size(0)
{

}

VirtualFile::~VirtualFile() {
    for (size_t i = 0 ; i < data.size() ; i++) {
        delete data[i];
    }
}

const char* VirtualFile::get_name()
{
    return name.c_str();
}

FileHandle* VirtualFile::open()
{
    return new VirtualFileHandle(this);
}

File* VirtualFile::create(const char* name, u32 flags)
{
    UNUSED(name);
    UNUSED(flags);

    return NULL;
}

File* VirtualFile::bind(File* child)
{
    UNUSED(child);

    return NULL;
}