#include "filesystem.h"

using namespace fs;

Registry* fs::registry;

Registry::Registry() {
}

void Registry::register_filesystem_implementation(std::string type, filesystem_factory_cb cb) {
    _factories[type] = cb;
}

bool Registry::create_filesystem(std::string type, fs::File* root, size_t offset, size_t length) {
    if (_factories.count(type) > 0) {
        return _factories[type](root, offset, length);
    }

    return false;
}