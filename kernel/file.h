#ifndef __OLLIOS_FILE_H
#define __OLLIOS_FILE_H

#include "types.h"

#define FILE_CREATE_DIR 0x1

namespace fs {

    void init();

    class File;

    class FileHandle {
    public:
        virtual ~FileHandle() {};
        virtual void close() = 0;

        virtual i32 write(const void* data, size_t count) = 0;
        virtual i32 read(void* buffer, size_t size) = 0;
        virtual i32 seek(i32 pos, size_t dir) = 0;

        virtual File* next_child() = 0;
        virtual void reset_child_iterator() = 0;
    };

    class File {
    public:
        virtual ~File() {};
        virtual FileHandle* open() = 0;

        virtual const char* get_name() = 0;
        virtual size_t get_size() = 0;

        virtual File* create(const char* name, u32 flags) = 0;
        virtual File* bind(File* child) = 0;

        File* get(const char* name);

        template<class T>
        T read() {
            T t;

            size_t total = 0;
            size_t ret;

            FileHandle* h = open();

            do {
                ret = h->read(((u8*)&t) + total, sizeof(T) - total);
                total += ret;
            } while (ret > 0);

            h->close();

            return t;
        }
    };

    extern File* root;
}

#endif