#include "fileview.h"

using namespace fs;

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
