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

    template <class T, class C>
    struct ReadWriteDataContext {
        C* context;
        T (*getter)(C* context);
        void (*setter)(C* context, T value);
    };
    
    template <class T>
    struct ReadOnlyDataContext {
        T data;
    };

    class InterfaceFile : public File {
    public:
        typedef i32 (*SetValue)(const char* value, size_t length,  size_t pos, void* context);
        typedef i32 (*GetValue)(char* buffer, size_t length, size_t pos, void* context);

        InterfaceFile(const std::string& name, SetValue setter, GetValue getter, void* context, void* context2 = nullptr, bool positionIndependant = true);
        ~InterfaceFile();

        const char* get_name();

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

        std::string name;
        SetValue setter;
        GetValue getter;
        void* context;
        void* context2;
        bool positionIndependant;

        template<size_t LENGTH>
        static InterfaceFile* read_only(const std::string& name, const void* data) {
            return new InterfaceFile(name, [](const char* value, size_t length, size_t pos, void* context)->i32 {
                (void) value;
                (void) length;
                (void) context;
                (void) pos;

                return 0;
            }, [](char* buffer, size_t length, size_t pos, void* context)->i32 {
                (void) pos;

                if (length < LENGTH) {
                    return 0;
                } else {
                    memcpy(buffer, (char *)context, length);
                    return LENGTH;
                }
            }, (void*) data);
        }

        static InterfaceFile* read_only_string(const std::string& name, const char* data) {
            return new InterfaceFile(name, [](const char* value, size_t length, size_t pos, void* context)->i32 {
                (void) value;
                (void) length;
                (void) context;
                (void) pos;

                return 0;
            }, [](char* buffer, size_t length, size_t pos, void* context)->i32 {
                (void) pos;

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

        template <class T>
        static InterfaceFile* read_only_data(const std::string& name, T data) {

            struct ReadOnlyDataContext<T>* context = new struct ReadOnlyDataContext<T>;
            context->data = data;

            return new InterfaceFile(name, [](const char* value, size_t length, size_t pos, void* context)->i32 {
                (void) value;
                (void) length;
                (void) context;
                (void) pos;

                return 0;
            }, [](char* buffer, size_t length, size_t pos, void* contextRaw)->i32 {
                (void) pos;

                struct ReadOnlyDataContext<T>* context = (ReadOnlyDataContext<T>*) contextRaw;
                char data[12];

                sprintf(data, "%d\n", context->data);
                size_t source_length = strlen(data);
                if (length <= source_length) {
                    return 0;
                } else {
                    strcpy(buffer, data);
                    return source_length;
                }
            }, (void*) context);
        }

        template <class T, class C>
        static InterfaceFile* read_write_data(const std::string& name, C* context, T (*getter)(C* context), void (*setter)(C* context, T value)) {
            
            struct ReadWriteDataContext<T, C>* fileContext = new struct ReadWriteDataContext<T, C>;
            fileContext->context = context;
            fileContext->setter = setter;
            fileContext->getter = getter;

            return new InterfaceFile(name, [](const char* value, size_t length, size_t pos, void* rawContext)->i32 {
                struct ReadWriteDataContext<T, C>* context = (struct ReadWriteDataContext<T, C>*) rawContext;
                T parsed = T();

                bool parsed_anything = false;
                bool finished = false;
                bool is_min = false;

                for (size_t i = 0 ; i < length ; i++) {
                    char c = value[i];
                    if (c == '-') {
                        if (parsed_anything) {
                            return 0;
                        }
                        parsed_anything = true;
                        is_min = true;
                    } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                        if (parsed_anything) {
                            finished = true;
                        }
                        continue;
                    } else if (c >= '0' && c <= '9') {
                        if (finished) {
                            return 0;
                        }
                        parsed_anything = true;
                        parsed *= 10;
                        parsed += c - '0';
                    } else {
                        return 0;
                    }
                }

                if (is_min) {
                    parsed = -parsed;
                }

                context->setter(context->context, parsed);

                return length;
            }, [](char* buffer, size_t length, size_t pos, void* rawContext)->i32 {
                struct ReadWriteDataContext<T, C>* context = (struct ReadWriteDataContext<T, C>*) rawContext;

                char data[12];

                sprintf(data, "%d\n", context->getter(context->context));
                size_t source_length = strlen(data);
                if (length <= source_length) {
                    return 0;
                } else {
                    strcpy(buffer, data);
                    return source_length;
                }
            }, (void*) fileContext);
        }
    };

}

#endif