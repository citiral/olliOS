#include "types.h"
#include "string.h"
#include "kstd/vector.h"
#include "bindings.h"
#include <stdio.h>

using namespace bindings;

#define RAMBINDING_ENTRY_SIZE 0x1000

size_t on_read_callback(OwnedBinding* binding, void* buffer, size_t size, size_t offset);
bool on_write_callback(OwnedBinding* binding, size_t size, const void* data);

class RamBinding: public bindings::OwnedBinding {
public:
    RamBinding(std::string name): OwnedBinding(name), data(), size(0) {
        on_read(on_read_callback);
        on_write(on_write_callback);
    }
    std::vector<u8*> data;
    size_t size;
};

size_t on_read_callback(OwnedBinding* binding, void* buffer, size_t size, size_t offset) {
    RamBinding* file = (RamBinding*) binding;

    size_t index = offset / RAMBINDING_ENTRY_SIZE;
    if (index >= file->data.size())
        return 0;

    u8* source = file->data[index];
    if ((offset % RAMBINDING_ENTRY_SIZE) + size > RAMBINDING_ENTRY_SIZE) {
        size = RAMBINDING_ENTRY_SIZE - (offset % RAMBINDING_ENTRY_SIZE);
    }

    memcpy(buffer, source, size);
    return size;
}

bool on_write_callback(OwnedBinding* binding, size_t size, const void* data)
{
    return false;
}

extern "C" void module_load(Binding* root, const char* argv)
{
    std::string fullarg = argv + strlen("ramfs.so ");
    
    while (fullarg[fullarg.size() - 1] == '\n' || fullarg[fullarg.size() - 1] == ' ')
        fullarg = fullarg.substr(0, fullarg.size() - 1);

    std::string path = bindings::get_path(fullarg);
    std::string filename = bindings::get_filename(fullarg);

    
    bindings::Binding* parent = root->get(path.c_str());

    if (parent != nullptr)
        parent->add(new RamBinding(filename.c_str()));
}
