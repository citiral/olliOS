#ifndef __OLIOS_STREAM_H
#define __OLIOS_STREAM_H

#include "file.h"
#include "kstd/string.h"
#include "kstd/shared_ptr.h"
#include "threading/mutex.h"
#include "threading/thread.h"
#include "threading/waiting_list.h"

namespace fs {
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