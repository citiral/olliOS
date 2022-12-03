#ifndef __OLLIOS_FILESYSTEM_H
#define __OLLIOS_FILESYSTEM_H

#include "file.h"
#include "kstd/string.h"
#include "kstd/unordered_map.h"

namespace  fs {
    class Registry {
    public:
        typedef bool (*filesystem_factory_cb)(fs::File* root, size_t offset, size_t length);

        Registry();

        void register_filesystem_implementation(std::string type, filesystem_factory_cb cb);
        bool create_filesystem(std::string type, fs::File* root, size_t offset, size_t length);

    private:
        std::unordered_map<std::string, filesystem_factory_cb> _factories;
    };

    extern Registry* registry;
}

#endif