#ifndef __STREAM_H
#define __STREAM_H

#include <stddef.h>

/*
* The stream will be the base of a lot of classes in the operating system.
* Much like many other operating systems all device drivers, files,.. will
* be represented as simple data streams.
*/

class Stream {
public:
    virtual ~Stream();

    //writes size_t bytes to the stream, starting from data. Returns the
    //amount of bytes actually written.
    virtual size_t write(const void* data, size_t amount) = 0;
    //writes bytes to the stream untill a newline is encountered. Returns the
    //amount of bytes actually written.
    virtual size_t write(const void* data) = 0;
    //reads size_t bytes from the device and inserts it into data.
    //returns the amount of bytes actually read.
    virtual size_t read(void* data, size_t amount) = 0;
};

#endif
