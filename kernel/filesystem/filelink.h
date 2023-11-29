#ifndef __OLLIOS_FILELINK_H
#define __OLLIOS_FILELINK_H

#include "file.h"
#include "kstd/string.h"

namespace fs {
    class FileLink : public File {
    public:
        FileLink(std::string name, File* original);

        const char* get_name();

        FileHandle* open();

        File* create(const char* name, u32 flags);
        File* bind(File* child);

    private:
        std::string _name;
        File* _original;
    };
}

#endif