#ifndef __OLLIOS_VIRTUALFILE_H
#define __OLLIOS_VIRTUALFILE_H

#include "file.h"
#include "kstd/string.h"
#include "kstd/vector.h"

//#define VIRTUAL_FILE_SMALL_DATA_SIZE 8
#define VIRTUAL_FILE_CHUNK_SIZE 128

namespace fs {

    class VirtualFile;
    class VirtualFolder;
    class Stream;
    class ChunkedStream;

    class VirtualFileHandle : public FileHandle {
    public:
        VirtualFileHandle(VirtualFile* file);
        void close();

        i32 write(const void* data, size_t count);
        i32 read(void* buffer, size_t size);
        i32 seek(i32 pos, size_t dir);

        File* next_child();
        void reset_child_iterator();

    private:
        size_t _offset;
        VirtualFile* _file;
    };


    class VirtualFolderHandle : public FileHandle {
    public:
        VirtualFolderHandle(VirtualFolder* file);
        void close();

        i32 write(const void* data, size_t count);
        i32 read(void* buffer, size_t size);
        i32 seek(i32 pos, size_t dir);

        File* next_child();
        void reset_child_iterator();

    private:
        size_t _child_offset;
        VirtualFolder* _file;
    };


    class VirtualFile : public File {
    public:
        VirtualFile(std::string name);
        ~VirtualFile();

        const char* get_name();
        size_t get_size();

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;

        //uint8_t small_data[VIRTUAL_FILE_SMALL_DATA_SIZE];
        std::vector<uint8_t*> data;
        size_t size;
    };


    class VirtualFolder : public File {
    public:
        VirtualFolder(std::string name);

        const char* get_name();
        size_t get_size();

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;
        std::vector<File*> children;
    };


    class StreamHandle : public FileHandle {
    public:
        StreamHandle(Stream* stream);
        void close();

        i32 write(const void* data, size_t count);
        i32 read(void* buffer, size_t size);
        i32 seek(i32 pos, size_t dir);

        File* next_child();
        void reset_child_iterator();

    protected:
        Stream* _stream;
    };

    class Stream : public File {
    public:
        Stream(std::string name, size_t size);
        ~Stream();

        const char* get_name();
        size_t get_size();

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;

        uint8_t* data;
        size_t size;
        size_t read;
        size_t write;
    };

    class ChunkedStreamHandle : public StreamHandle {
    public:
        ChunkedStreamHandle(ChunkedStream* stream);

        i32 write(const void* data, size_t count);
        i32 read(void* buffer, size_t size);
    };

    class ChunkedStream : public Stream {
    public:
        ChunkedStream(std::string name, size_t size, size_t chunk_size);

        FileHandle* open();

        size_t chunk_size;
    };

}

#endif