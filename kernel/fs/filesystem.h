//
// Created by Olivier on 01/10/16.
//

#ifndef OLLIOS_GIT_FILESYSTEM_H
#define OLLIOS_GIT_FILESYSTEM_H

#include "../streams/stream.h"
#include "../util/linkedlist.h"
#include <vector>

enum class FileType {
    FILE = 1,
    FOLDER = 2,
};

using File = Stream;

class FileDescriptor {
    virtual ~FileDescriptor() {};

    // returns the full name of the file. This is the full path including the directory
    virtual const char* getFullName() const = 0;

    // returns the type of the item this descriptor describes.
    virtual FileType getFileType() = 0;

    // opens the file/folder so we can read/write to the file/folder. TODO: make sure a file can be Write opened only once
    virtual File* open() = 0;
};

/*
 * Abstract filesystem. This provides an API that can be implemented by physical and virtual filesystems.
 */
class FileSystem {
    virtual ~FileDescriptor() {};
    virtual FileDescriptor* getFile(const char* dir) = 0;
    virtual std::vector<const char*> getFolderContents() = 0;
};

extern FileSystem* rootFileSystem;

#endif //OLLIOS_GIT_FILESYSTEM_H
