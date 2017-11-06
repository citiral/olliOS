#ifndef OLLIOS_BLOCK_DEVICE_H
#define OLLIOS_BLOCK_DEVICE_H

#include "kstd/cstddef.h"
#include "devices/device.h"
#include "types.h"

#define STREAM_SIZE_UNDEFINED -1

class BlockDevice : public Device {
public:
    virtual ~BlockDevice();

    // writes size_t bytes to the stream, starting from data. Returns the
    // amount of bytes actually written.
    virtual size_t write(const void* data, size_t amount) = 0;
    // writes bytes to the stream until a null terminator is encountered. Returns the
    // amount of bytes actually written.
    virtual size_t write(const void* data) = 0;
    // writes a single byte to the stream, Returns the amount of bytes written (0 or 1)
    virtual size_t write(char data) = 0;
    // reads size_t bytes from the device and inserts it into data.
    // returns the amount of bytes actually read.
    virtual size_t read(void* data, size_t amount) = 0;
    // seeks in the stream, changing the current location in regards to
    // position, with an added offset.
    // if position == SeekType.BEGIN, the file will seek starting from the
    // beginning. Negative offets are UNDEFINED BEHAVIOUR.
    // if position == SeekType.END, the file will seek starting from the
    // beginning. Positive offets are UNDEFINED BEHAVIOUR.
    // if position == SeekType.CURRENT, the file will seek starting from current
    // position.
    virtual size_t seek(i32 offset, int position) = 0;
};

#endif