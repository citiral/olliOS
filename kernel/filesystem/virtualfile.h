#ifndef __OLLIOS_VIRTUALFILE_H
#define __OLLIOS_VIRTUALFILE_H

#include "file.h"
#include "kstd/string.h"
#include "kstd/vector.h"
#include "threading/mutex.h"
#include "threading/waiting_list.h"

//#define VIRTUAL_FILE_SMALL_DATA_SIZE 8
#define VIRTUAL_FILE_CHUNK_SIZE 512

namespace fs {

    class VirtualFile;
    class Stream;
    class ChunkedStream;

    class VirtualFileHandle : public FileHandle {
    public:
        VirtualFileHandle(VirtualFile* file);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
        size_t get_size();


        File* next_child();
        void reset_child_iterator();

    private:
        VirtualFile* _file;
    };


    class VirtualFile : public File {
    public:
        VirtualFile(std::string name);
        ~VirtualFile();

        const char* get_name();
        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;

        //uint8_t small_data[VIRTUAL_FILE_SMALL_DATA_SIZE];
        std::vector<uint8_t*> data;
        size_t size;
    };

}

#endif