#include "chunkedstream.h"

using namespace fs;

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