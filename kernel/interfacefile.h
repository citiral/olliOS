#ifndef __OLIOS_INTERFACE_FILE_H
#define __OLIOS_INTERFACE_FILE_H

#include "file.h"
#include "kstd/string.h"
#include <string.h>

namespace fs {

    class InterfaceFile;

    class InterfaceFileHandle : public FileHandle {
    public:
        InterfaceFileHandle(InterfaceFile* file);

        virtual i32 write(const void* buffer, size_t size, size_t pos);
        virtual i32 read(void* buffer, size_t size, size_t pos);
        size_t get_size();

        File* next_child();
        void reset_child_iterator();

    private:
        InterfaceFile* _file;
        int offset;
    };


    class InterfaceFile : public File {
    public:
        typedef i32 (*SetValue)(const char* value, size_t length, void* context);
        typedef i32 (*GetValue)(char* buffer, size_t length, void* context);

        InterfaceFile(const std::string& name, SetValue setter, GetValue getter, void* context);
        ~InterfaceFile();

        const char* get_name();

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;
        SetValue setter;
        GetValue getter;
        void* context;

        template<size_t LENGTH>
        static InterfaceFile* read_only(const std::string& name, const void* data) {
            return new InterfaceFile(name, [](const char* value, size_t length, void* context)->i32 {
                (void) value;
                (void) length;
                (void) context;

                return 0;
            }, [](char* buffer, size_t length, void* context)->i32 {
                if (length < LENGTH) {
                    return 0;
                } else {
                    memcpy(buffer, (char *)context, length);
                    return LENGTH;
                }
            }, (void*) data);
        }

        static InterfaceFile* read_only_string(const std::string& name, const char* data) {
            return new InterfaceFile(name, [](const char* value, size_t length, void* context)->i32 {
                (void) value;
                (void) length;
                (void) context;

                return 0;
            }, [](char* buffer, size_t length, void* context)->i32 {
                const char* str = (const char*) context;
                size_t source_length = strlen(str);
                if (length <= source_length) {
                    return 0;
                } else {
                    strcpy(buffer, str);
                    return source_length;
                }
            }, (void*) data);
        }
    };

}

#endif