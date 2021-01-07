#include "interfacefile.h"

using namespace fs;

InterfaceFileHandle::InterfaceFileHandle(InterfaceFile* file): _file(file), offset(0) {

}

void InterfaceFileHandle::close() {
    delete this;
}

i32 InterfaceFileHandle::write(const void* data, size_t count) {
    if (_file->setter && offset == 0) {
        offset = _file->setter((const char*) data, count, _file->context);
        return offset;
    } else
        return 0;
}

i32 InterfaceFileHandle::read(void* buffer, size_t size) {
    if (_file->getter && offset == 0) {
        offset = _file->getter((char*) buffer, size, _file->context);
        return offset;
    } else
        return 0;
}

i32 InterfaceFileHandle::seek(i32 pos, size_t dir) {

    if (dir == SEEK_SET && pos == 0) {
        offset = 0;
    }

    return offset;
}
File* InterfaceFileHandle::next_child() {
    return nullptr;
}

void InterfaceFileHandle::reset_child_iterator() {
    
}

fs::InterfaceFile::InterfaceFile(const std::string& name, SetValue setter, GetValue getter, void* context): name(name), setter(setter), getter(getter), context(context) {
}

InterfaceFile::~InterfaceFile() {

}

const char* InterfaceFile::get_name() {
    return name.c_str();
}

size_t InterfaceFile::get_size() {
    return 0;
}

FileHandle* InterfaceFile::open() {
    return new InterfaceFileHandle(this);
}

File* InterfaceFile::create(const char* name, u32 flags) {
    (void) name;
    (void) flags;

    return nullptr;
}

File* InterfaceFile::bind(File* child) {
    (void) child;

    return nullptr;
}