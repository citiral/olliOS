#include "interfacefile.h"

using namespace fs;

InterfaceFileHandle::InterfaceFileHandle(InterfaceFile* file): _file(file), offset(0) {

}

i32 InterfaceFileHandle::write(const void* buffer, size_t size, size_t pos) {
    if (pos == 0)
        return _file->setter((const char*) buffer, size, pos, _file->context);
    else
        return 0;
}

i32 InterfaceFileHandle::read(void* buffer, size_t size, size_t pos) {
    if (pos == 0)
        return _file->getter((char*) buffer, size, pos, _file->context);
    else
        return 0;
}

File* InterfaceFileHandle::next_child() {
    return nullptr;
}

void InterfaceFileHandle::reset_child_iterator() {
    
}

size_t InterfaceFileHandle::get_size() {
    return 0;
}

fs::InterfaceFile::InterfaceFile(const std::string& name, SetValue setter, GetValue getter, void* context, void* context2): name(name), setter(setter), getter(getter), context(context), context2(context2) {
}

InterfaceFile::~InterfaceFile() {

}

const char* InterfaceFile::get_name() {
    return name.c_str();
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