//
// Created by Olivier on 01/10/16.
//

#ifndef OLLIOS_GIT_FILESYSTEM_H
#define OLLIOS_GIT_FILESYSTEM_H

#include "streams/stream.h"
#include "kstd/string.h"

enum DirEntryType {
    Folder,
    File,
};

class DirEntry {
public:
    DirEntry() {};
    DirEntry(DirEntry&) = delete;
    DirEntry(DirEntry&&) = delete;

    DirEntry& operator=(DirEntry&) = delete;
    DirEntry& operator=(DirEntry&&) = delete;

    virtual ~DirEntry() {};
    virtual bool valid() = 0;
    virtual bool advance() = 0;
    virtual std::string name() = 0;
    virtual DirEntryType type() = 0;

    virtual DirEntry* openDir() = 0;
    virtual Stream* openFile() = 0;
};

class FileSystem {
public:
    virtual DirEntry* getRoot() = 0;
};

extern FileSystem* rootFileSystem;

#endif //OLLIOS_GIT_FILESYSTEM_H
