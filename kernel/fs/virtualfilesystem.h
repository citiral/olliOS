#ifndef __VIRTUALFILESYSTEM_H
#define __VIRTUALFILESYSTEM_H

#include "fs/filesystem.h"
#include "kstd/vector.h"
#include "kstd/string.h"

class VirtualDirectory {
public:
    std::string name;
    std::vector<std::pair<std::string, FileSystem*>> fs;
    std::vector<VirtualDirectory*> directories;
    std::vector<std::pair<std::string, Stream*>> files;

    VirtualDirectory* createChildDirectory(std::string name);
    VirtualDirectory* getChildDirectory(const char* name);
private:
};

class VirtualDirEntry : public DirEntry {
public:
    VirtualDirEntry(VirtualDirectory* dir);
    virtual ~VirtualDirEntry();

    virtual bool valid();
    virtual bool advance();
    virtual std::string name();
    virtual DirEntryType type();
    virtual DirEntry* openDir();
    virtual Stream* openFile();

private:
    // The directory this is iterating over
    VirtualDirectory* _vdir;

    // determines the index in said list 
    int _index;
};

class VirtualFileSystem : public FileSystem {
public:
    VirtualFileSystem();
    ~VirtualFileSystem();

    void BindFilesystem(std::string name, FileSystem* fs);
    
    DirEntry* getRoot();

private:
    VirtualDirectory _root;
};

extern VirtualFileSystem* vfs;

#endif