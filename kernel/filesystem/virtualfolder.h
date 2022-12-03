#ifndef __OLLIOS_VIRTUALFOLDER_H
#define __OLLIOS_VIRTUALFOLDER_H

#include "file.h"
#include "kstd/string.h"
#include "kstd/vector.h"
namespace fs {
    class VirtualFolder : public File {
    public:
        VirtualFolder(std::string name);

        const char* get_name();

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;
        std::vector<File*> children;
    };

    class VirtualFolderHandle : public FileHandle {
    public:
        VirtualFolderHandle(VirtualFolder* file);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
        size_t get_size();


        File* next_child();
        void reset_child_iterator();

    private:
        size_t _child_offset;
        VirtualFolder* _file;
    };
}

#endif