#include "streams/memorystream.h"
#include "cdefs.h"
#include <stdio.h>
#include <string.h>

MemoryStream::MemoryStream(void* data, size_t length): _data((char*)data), _length(length), _pos(0) {

}

MemoryStream::~MemoryStream() {
    delete[] _data;
}

DeviceType MemoryStream::getDeviceType() const
{
	return DeviceType::Memory;
}

void MemoryStream::getDeviceInfo(void* deviceinfo) const
{
	UNUSED(deviceinfo);
	CPU::panic("Can't execute getDeviceInfo of MemoryStream");
}

size_t MemoryStream::write(const void* data, size_t amount) {
    if (amount + _pos > _length) {
        amount = _length - _pos;
    }

    memcpy(_data + _pos, data, amount);
    _pos += amount;

    return amount;
}

size_t MemoryStream::write(const void* data) {
	size_t added = 0;
	// This conversion is done because you're not allowed to increment a const void* in C++11
	const char* cdata = (char*) data;
    while (*(u8*)data != 0 && _pos < _length) {
        _data[_pos++] = *((u8*)(cdata++));
        added++;
    }

    return added;
}

size_t MemoryStream::write(char data) {
    if (_pos < _length) {
        _data[_pos++] = data;
        return 1;
    } else {
        return 0;
    }
}

size_t MemoryStream::read(void* data, size_t amount) {
    if (amount + _pos > _length) {
        amount = _length - _pos;
    }

    memcpy(data, _data + _pos, amount);
    _pos += amount;

    return amount;
}

size_t MemoryStream::seek(i32 offset, int position) {
    if (offset == SEEK_SET) {
        if ((unsigned int) position > _length) {
            position = _length;
        }
    } else if (offset == SEEK_CUR) {
        position = _pos + position;
        if (position > 0 && (unsigned int) position > _length) {
            position = _length;
        } else if (position < 0) {
            position = 0;
        }
    } else if (offset == SEEK_END) {
        if ((unsigned int) (-position) > _length) {
            position = -_length;
        }
        position = _length - position;
    }

    _pos = position;

    return _pos;
}