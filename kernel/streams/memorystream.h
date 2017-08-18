#ifndef _MEMORY_STREAM_H
#define _MEMORY_STREAM_H

#include "kstd/cstddef.h"
#include "types.h"
#include "streams/stream.h"

class MemoryStream: public Stream {
public:
    MemoryStream(void* data, size_t length);
    virtual ~MemoryStream();

    virtual size_t write(const void* data, size_t amount);
    virtual size_t write(const void* data);
    virtual size_t write(char data);
    virtual size_t read(void* data, size_t amount);
    virtual size_t seek(i32 offset, int position);

private:
    char* _data;
    size_t _length;
    size_t _pos;
};

#endif
