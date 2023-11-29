#include "filelink.h"

namespace fs {

FileLink::FileLink(std::string name, File* original)
    : _name(name), _original(original) {
}

const char* FileLink::get_name() {
    return _name.c_str();
}

FileHandle* FileLink::open() {
    return _original->open();
}

File* FileLink::create(const char* name, u32 flags) {
    return _original->create(name, flags);
}

File* FileLink::bind(File* child) {
    return _original->bind(child);
}

}
