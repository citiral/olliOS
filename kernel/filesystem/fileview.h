#ifndef __OLIOS_FILEVIEW_H
#define __OLIOS_FILEVIEW_H

#include "file.h"
#include "kstd/string.h"

namespace fs {
    class FileViewHandle : public FileHandle {
    public:        
        FileViewHandle(FileHandle* handle, size_t start, size_t length);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
        size_t get_size();

        File* next_child();
        void reset_child_iterator();

    private:
        FileHandle* _handle;
        size_t _start;
        size_t _length;
    };

    class FileView : public File {
    public:
        FileView(std::string name, File* parent, size_t start, size_t length);
        ~FileView();

        const char* get_name();
        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

    private:
        std::string _name;
        File* _parent;
        size_t _start;
        size_t _length;
    };
}

#endif