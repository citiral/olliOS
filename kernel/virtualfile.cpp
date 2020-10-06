#include "virtualfile.h"
#include "cdefs.h"
#include "threading/thread.h"

using namespace fs;

VirtualFileHandle::VirtualFileHandle(VirtualFile* file): _offset(0), _file(file)
{

}

void VirtualFileHandle::close()
{
    delete this;
}

i32 VirtualFileHandle::write(const void* buffer, size_t count)
{
    size_t chunk = _offset / VIRTUAL_FILE_CHUNK_SIZE;
    size_t chunk_offset = _offset % VIRTUAL_FILE_CHUNK_SIZE;
    size_t chunk_remaining = VIRTUAL_FILE_CHUNK_SIZE - chunk_offset;

    i32 written = 0;

    while (count > 0) {
        if (chunk == _file->data.size()) {
            _file->data.push_back(new uint8_t[VIRTUAL_FILE_CHUNK_SIZE]);
        }

        size_t length = count < chunk_remaining ? count : chunk_remaining;
        memcpy(_file->data[chunk] + chunk_offset, buffer, length);

        chunk++;
        chunk_offset = 0;
        chunk_remaining = VIRTUAL_FILE_CHUNK_SIZE;
        count -= length;

        buffer = ((uint8_t*)buffer) + length;
        written += length;
    }

    _file->size += written;
    _offset += written;

    return written;
}

i32 VirtualFileHandle::read(void* buffer, size_t size)
{
    if (_offset >= _file->size) {
        return 0;
    } else if (_offset + size >= _file->size) {
        size = _file->size - _offset;
    }

    i32 read = 0;

    size_t chunk = _offset / VIRTUAL_FILE_CHUNK_SIZE;
    size_t chunk_offset = _offset % VIRTUAL_FILE_CHUNK_SIZE;
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

    _offset += read;

    return read;
}

i32 VirtualFileHandle::seek(i32 pos, size_t dir)
{
    i64 new_offset = 0;
    if (dir == SEEK_SET) {
        new_offset = pos;
    } else if (dir == SEEK_CUR) {
        new_offset = _offset;
        new_offset += pos;
    } else if (dir == SEEK_END) {
        new_offset = _file->size;
        new_offset -= pos;
    }

    if (new_offset < 0) {
        _offset = 0;
    } else if (new_offset > _file->size) {
        _offset = _file->size;
    } else {
        _offset = (size_t) new_offset;
    }

    return _offset;
}

File* VirtualFileHandle::next_child()
{
    return NULL;
}

void VirtualFileHandle::reset_child_iterator()
{
}


VirtualFolderHandle::VirtualFolderHandle(VirtualFolder* file): _child_offset(0), _file(file)
{

}

void VirtualFolderHandle::close()
{
    delete this;
}

i32 VirtualFolderHandle::write(const void* data, size_t count)
{
    UNUSED(data);
    UNUSED(count);

    return -1;
}

i32 VirtualFolderHandle::read(void* buffer, size_t size)
{
    UNUSED(buffer);
    UNUSED(size);

    return -1;
}

i32 VirtualFolderHandle::seek(i32 pos, size_t dir)
{
    UNUSED(pos);
    UNUSED(dir);

    return -1;
}

File* VirtualFolderHandle::next_child()
{
    if (_child_offset >= _file->children.size()) {
        return NULL;
    } else {
        _child_offset++;
        return _file->children[_child_offset - 1];
    }
}

void VirtualFolderHandle::reset_child_iterator()
{
    _child_offset = 0;
}

VirtualFile::VirtualFile(std::string name): name(name), data()
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

size_t VirtualFile::get_size()
{
    return size;
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

VirtualFolder::VirtualFolder(std::string name): name(name), children()
{

}

const char* VirtualFolder::get_name()
{
    return name.c_str();
}

size_t VirtualFolder::get_size()
{
    return 0;
}

FileHandle* VirtualFolder::open()
{
    return new VirtualFolderHandle(this);
}

File* VirtualFolder::create(const char* name, u32 flags)
{
    if (flags & FILE_CREATE_DIR) {
        return bind(new VirtualFolder(name));
    } else {
        return bind(new VirtualFile(name));
    }
}

File* VirtualFolder::bind(File* child)
{
    children.push_back(child);
    return child;
}

StreamHandle::StreamHandle(Stream* stream): _stream(stream)
{            
}

void StreamHandle::close()
{
    delete this;
}

i32 StreamHandle::write(const void* _data, size_t count)
{
    const uint8_t* data = (const uint8_t*) _data;

    size_t size = _stream->size;
    size_t i;
    for (i = 0 ; i < count ; i++) {
        size_t next_write = (_stream->write + 1) < size ? _stream->write + 1 : 0;

        if (next_write == _stream->read) {
            break;
        }

        _stream->data[_stream->write] = data[i];
        _stream->write = next_write;
    }

    return i;
}

i32 StreamHandle::read(void* _data, size_t count)
{
    uint8_t* data = (uint8_t*) _data;

    size_t size = _stream->size;
    size_t i;

    for (i = 0 ; i < count ; i++) {
        if (_stream->read == _stream->write) {
            break;
        }

        size_t next_read = (_stream->read + 1) < size ? _stream->read + 1 : 0;

        data[i] = _stream->data[_stream->read];
        _stream->read = next_read;
    }

    return i;
}

i32 StreamHandle::seek(i32 pos, size_t dir)
{
    UNUSED(pos);
    UNUSED(dir);

    return -1;
}

File* StreamHandle::next_child()
{
    return NULL;
}

void StreamHandle::reset_child_iterator()
{
}


Stream::Stream(std::string name, size_t size): name(name), size(size)
{
    if (size <= 1) {
        size = 2;
    }

    data = new uint8_t[size];
    write = 0;
    read = 0;
}

Stream::~Stream()
{
    delete data;
}

const char* Stream::get_name()
{
    return name.c_str();
}

size_t Stream::get_size()
{
    if (read < write) {
        return write - read;
    } else {
        return write + (size - read);
    }
}

FileHandle* Stream::open()
{
    return new StreamHandle(this);
}

File* Stream::create(const char* name, u32 flags)
{
    UNUSED(name);
    UNUSED(flags);

    return nullptr;
}

File* Stream::bind(File* child)
{
    UNUSED(child);

    return nullptr;
}

ChunkedStreamHandle::ChunkedStreamHandle(ChunkedStream* stream): StreamHandle(stream)
{
    
}

i32 ChunkedStreamHandle::write(const void* _data, size_t count)
{
    ChunkedStream* cstream = (ChunkedStream*) _stream;
    const uint8_t* data = (const uint8_t*)_data;

    count -= (count % cstream->chunk_size);

    for (size_t i = 0 ; i < count ; ) {
        i32 write = StreamHandle::write(data + i, count - i);
        if (write == 0) {
            threading::exit();
        }
        i += write;
    }

    return count;
}

i32 ChunkedStreamHandle::read(void* _data, size_t count)
{
    ChunkedStream* cstream = (ChunkedStream*) _stream;
    uint8_t* data = (uint8_t*)_data;

    count -= (count % cstream->chunk_size);

    for (size_t i = 0 ; i < count ; ) {
        i32 read = StreamHandle::read(data + i, count - i);
        if (read == 0) {
            threading::exit();
        }
        i += read;
    }
    return count;
}

ChunkedStream::ChunkedStream(std::string name, size_t size, size_t chunk_size): Stream(name, size), chunk_size(chunk_size)
{

}

FileHandle* ChunkedStream::open()
{
    return new ChunkedStreamHandle(this);
}