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

class DirEntry {
public:
    virtual ~DirEntry() {};
    virtual bool valid() = 0;
    virtual bool advance() = 0;
    virtual const char* name() = 0;
    virtual DirEntryType type() = 0;
};

class FileSystem {
public:
    virtual DirEntry* openDir(const char* path) = 0;
    virtual Stream* openFile(const char* path) = 0;
};

extern FileSystem* rootFileSystem;

#endif //OLLIOS_GIT_FILESYSTEM_H
