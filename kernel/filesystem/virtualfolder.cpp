#include "virtualfolder.h"
#include "virtualfile.h"
#include "filelink.h"

using namespace fs;

VirtualFolderHandle::VirtualFolderHandle(VirtualFolder* file): _child_offset(0), _file(file)
{

}

i32 VirtualFolderHandle::write(const void* buffer, size_t size, size_t pos)
{
    UNUSED(buffer);
    UNUSED(size);
    UNUSED(pos);

    return -1;
}

i32 VirtualFolderHandle::read(void* buffer, size_t size, size_t pos)
{
    UNUSED(buffer);
    UNUSED(size);
    UNUSED(pos);

    return -1;
}

size_t VirtualFolderHandle::get_size()
{
    return 0;
}

File* VirtualFolderHandle::next_child()
{
    if (_child_offset >= _file->children.size()) {
        return NULL;
    } else {
        _child_offset++;
        return _file->children[_child_offset - 1];
    }
}

void VirtualFolderHandle::reset_child_iterator()
{
    _child_offset = 0;
}

VirtualFolder::VirtualFolder(std::string name): name(name), children()
{
    bind(new FileLink(".", this));
}

const char* VirtualFolder::get_name()
{
    return name.c_str();
}

FileHandle* VirtualFolder::open()
{
    return new VirtualFolderHandle(this);
}

File* VirtualFolder::create(const char* name, u32 flags)
{
    if (flags & FILE_CREATE_DIR) {
        auto folder = new VirtualFolder(name);
        folder->bind(new FileLink("..", this));
        return bind(folder);
    } else {
        return bind(new VirtualFile(name));
    }
}

File* VirtualFolder::bind(File* child)
{
    children.push_back(child);
    return child;
}