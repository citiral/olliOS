#ifndef __OLIOS_CHUNKEDSTREAM_H
#define __OLIOS_CHUNKEDSTREAM_H

#include "stream.h"

namespace fs {
    class ChunkedStream : public Stream {
    public:
        ChunkedStream(std::string name, size_t size, size_t chunk_size);

        FileHandle* open();

        size_t chunk_size;
    };
    
    class ChunkedStreamHandle : public StreamHandle {
    public:
        ChunkedStreamHandle(ChunkedStream* stream);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
    };
}

#endif