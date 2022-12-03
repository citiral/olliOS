#include "stream.h"

using namespace fs;

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

Stream::Stream(std::string name, size_t size): name(name), size(size), closed(0), waitingRead(), waitingWrite(), m()
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