//
// Created by Olivier on 01/10/16.
//

#ifndef OLLIOS_GIT_FILESYSTEM_H
#define OLLIOS_GIT_FILESYSTEM_H

#include "streams/stream.h"

enum DirEntryType {
    Folder,
    File,
};


class DirEntry : Stream {
public:
    virtual ~DirEntry();

    DirEntryType type;
};

class FileSystem {
public:
    virtual DirEntry* openDir(const char* path) = 0;
};

extern FileSystem* rootFileSystem;

#endif //OLLIOS_GIT_FILESYSTEM_H
