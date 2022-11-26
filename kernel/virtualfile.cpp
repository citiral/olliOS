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


VirtualFolderHandle::VirtualFolderHandle(VirtualFolder* file): _child_offset(0), _file(file)
{

}

i32 VirtualFolderHandle::write(const void* buffer, size_t size, size_t pos)
{
    UNUSED(buffer);
    UNUSED(size);
    UNUSED(pos);

    return -1;
}

i32 VirtualFolderHandle::read(void* buffer, size_t size, size_t pos)
{
    UNUSED(buffer);
    UNUSED(size);
    UNUSED(pos);

    return -1;
}

size_t VirtualFolderHandle::get_size()
{
    return 0;
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

VirtualFolder::VirtualFolder(std::string name): name(name), children()
{

}

const char* VirtualFolder::get_name()
{
    return name.c_str();
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

i32 StreamHandle::write(const void* buffer, size_t count, size_t pos)
{
    UNUSED(pos);

    const uint8_t* data = (const uint8_t*) buffer;

    _stream->m.lock();
    size_t size = _stream->size;
    size_t i;
    bool written = false;

    for (i = 0 ; i < count ; i++) {
        size_t next_write = (_stream->write + 1) < size ? _stream->write + 1 : 0;

        if (next_write == _stream->read) {
            if (written) {
                break;
            } else {
                bool eflags = CLI();
                _stream->waitingWrite.add_blocked_thread(threading::currentThread());
                _stream->m.release();
                STI(eflags);
                threading::exit();
                _stream->m.lock();
                if (_stream->closed) {
                    break;
                }
                i--;
                continue;
            }
        }

        _stream->data[_stream->write] = data[i];
        _stream->write = next_write;
        written = true;
    }

    _stream->m.release();
    _stream->waitingRead.unblock_all_threads();

    return i;
}

i32 StreamHandle::read(void* buffer, size_t count, size_t pos)
{
    UNUSED(pos);

    uint8_t* data = (uint8_t*) buffer;

    _stream->m.lock();
    size_t size = _stream->size;
    size_t i;
    bool read = false;

    for (i = 0 ; i < count ; i++) {
        if (_stream->read == _stream->write) {
            if (read || _stream->closed) {
                break;
            } else {
                bool eflags = CLI();
                _stream->waitingRead.add_blocked_thread(threading::currentThread());
                _stream->m.release();
                STI(eflags);
                threading::exit();

                _stream->m.lock();
                if (_stream->closed) {
                    break;
                }
                i--;
                continue;
            }
        }

        size_t next_read = (_stream->read + 1) < size ? _stream->read + 1 : 0;

        data[i] = _stream->data[_stream->read];
        _stream->read = next_read;
        read = true;
    }

    _stream->m.release();
    _stream->waitingWrite.unblock_all_threads();

    return i;
}

File* StreamHandle::next_child()
{
    return NULL;
}

void StreamHandle::reset_child_iterator()
{
}

size_t StreamHandle::get_size()
{
    _stream->m.lock();

    size_t ret = 0;
    if (_stream->read < _stream->write) {
        ret = _stream->write - _stream->read;
    } else {
        ret = _stream->write + (_stream->size - _stream->read);
    }

    _stream->m.release();

    return ret;
}


Stream::Stream(std::string name, size_t size): name(name), size(size), waitingRead(), waitingWrite(), closed(0), m()
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

i32 ChunkedStreamHandle::write(const void* buffer, size_t size, size_t pos)
{
    ChunkedStream* cstream = (ChunkedStream*) _stream;
    const uint8_t* data = (const uint8_t*)buffer;

    size -= (size % cstream->chunk_size);

    for (size_t i = 0 ; i < size ; ) {
        i32 write = StreamHandle::write(data + i, size - i, pos + i);
        if (write == 0) {
            threading::exit();
        }
        i += write;
    }

    return size;
}

i32 ChunkedStreamHandle::read(void* buffer, size_t size, size_t pos)
{
    ChunkedStream* cstream = (ChunkedStream*) _stream;
    uint8_t* data = (uint8_t*)buffer;

    size -= (size % cstream->chunk_size);

    for (size_t i = 0 ; i < size ; ) {
        i32 read = StreamHandle::read(data + i, size - i, pos + i);
        if (read == 0) {
            threading::exit();
        }
        i += read;
    }
    return size;
}

ChunkedStream::ChunkedStream(std::string name, size_t size, size_t chunk_size): Stream(name, size), chunk_size(chunk_size)
{

}

FileHandle* ChunkedStream::open()
{
    return new ChunkedStreamHandle(this);
}

OwnedStreamHandle::OwnedStreamHandle(std::shared_ptr<Stream> file, WriteMode mode): _file(file), _mode(mode), _handle(nullptr)
{
    _handle = _file->open();
}

OwnedStreamHandle::OwnedStreamHandle(const OwnedStreamHandle& handle)
{
    _file = handle._file;
    _mode = handle._mode;
    _handle = _file->open();
}

OwnedStreamHandle::~OwnedStreamHandle()
{
    _handle->close();
    if (_mode == WriteMode::Write) {
        _file->m.lock();
        _file->closed = true;
        _file->waitingRead.unblock_all_threads();
        _file->m.release();
    }
}

OwnedStreamHandle& OwnedStreamHandle::operator=(const OwnedStreamHandle& handle)
{
    _file = handle._file;
    _mode = handle._mode;
    _handle = _file->open();

    return *this;
}

i32 OwnedStreamHandle::write(const void* buffer, size_t size, size_t pos)
{
    if (_mode == WriteMode::Write) {
        return _handle->write(buffer, size, pos);
    } else {
        return -1;
    }
}

i32 OwnedStreamHandle::read(void* buffer, size_t size, size_t pos)
{

    if (_mode == WriteMode::Read) {
        return _handle->read(buffer, size, pos);
    } else {
        return -1;
    }
}

size_t OwnedStreamHandle::get_size()
{
    return _handle->get_size();
}

File* OwnedStreamHandle::next_child()
{
    return _handle->next_child();
}

void OwnedStreamHandle::reset_child_iterator()
{
    _handle->reset_child_iterator();
}


FileViewHandle::FileViewHandle(FileHandle* handle, size_t start, size_t length):
    _handle(handle), _start(start), _length(length)
{
}

i32 FileViewHandle::write(const void* buffer, size_t size, size_t pos)
{
    if (pos + size > _length) {
        size = _length - pos;
    }

    return _handle->write(buffer, size, pos + _start);
}

i32 FileViewHandle::read(void* buffer, size_t size, size_t pos)
{
    if (pos + size > _length) {
        size = _length - pos;
    }

    return _handle->read(buffer, size, pos + _start);
}

size_t FileViewHandle::get_size()
{
    return _length;
}

File* FileViewHandle::next_child()
{
    return _handle->next_child();
}

void FileViewHandle::reset_child_iterator()
{
    _handle->reset_child_iterator();
}

FileView::FileView(std::string name, File* parent, size_t start, size_t length):
    _name(name), _parent(parent), _start(start), _length(length)
{    
}
FileView::~FileView()
{
}

const char* FileView::get_name()
{
    return _name.c_str();
}

FileHandle* FileView::open()
{
    return new FileViewHandle(_parent->open(), _start, _length);
}

File* FileView::create(const char* name, u32 flags)
{
    return _parent->create(name, flags);
}

File* FileView::bind(File* child)
{
    return _parent->bind(child);
}
