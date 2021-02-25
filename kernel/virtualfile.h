#ifndef __OLLIOS_VIRTUALFILE_H
#define __OLLIOS_VIRTUALFILE_H

#include "file.h"
#include "kstd/string.h"
#include "kstd/vector.h"
#include "threading/mutex.h"
#include "threading/waiting_list.h"

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

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
        size_t get_size();


        File* next_child();
        void reset_child_iterator();

    private:
        VirtualFile* _file;
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


    class StreamHandle : public FileHandle {
    public:
        StreamHandle(Stream* stream);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
        size_t get_size();

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

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;

        uint8_t* data;
        size_t size;
        size_t read;
        size_t write;
        bool closed;
        threading::WaitingList waitingRead;
        threading::WaitingList waitingWrite;
        threading::Mutex m;
    };

    class ChunkedStreamHandle : public StreamHandle {
    public:
        ChunkedStreamHandle(ChunkedStream* stream);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
    };

    class ChunkedStream : public Stream {
    public:
        ChunkedStream(std::string name, size_t size, size_t chunk_size);

        FileHandle* open();

        size_t chunk_size;
    };

    class OwnedStreamHandle : public FileHandle {
    public:
        enum class WriteMode {
            Read,
            Write,
        };

        OwnedStreamHandle(std::shared_ptr<Stream> file, WriteMode mode);
        OwnedStreamHandle(const OwnedStreamHandle& handle);
        ~OwnedStreamHandle();

        OwnedStreamHandle& operator=(const OwnedStreamHandle& handle);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
        size_t get_size();

        File* next_child();
        void reset_child_iterator();

    private:
        std::shared_ptr<Stream> _file;
        WriteMode _mode;
        FileHandle* _handle;
    };

}

#endif