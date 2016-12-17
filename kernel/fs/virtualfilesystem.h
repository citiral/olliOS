//
// Created by Olivier on 01/10/16.
//

#ifndef OLLIOS_GIT_VIRTUALFILESYSTEM_H
#define OLLIOS_GIT_VIRTUALFILESYSTEM_H

#include "filesystem.h"

/*class VirtualFileDescriptorFile: FileDescriptor {
public:
    virtual ~VirtualFileDescriptor();

    virtual const char* getFullName() const = 0;
    virtual FileType getFileType() = 0;
    virtual File* open() = 0;
};

class VirtualFileDescriptorFolder: FileDescriptor {
public:
    virtual ~VirtualFileDescriptor();

    virtual const char* getFullName() const = 0;
    virtual FileType getFileType() = 0;
    virtual File* open() = 0;
};*/

enum VirtualNodeType {
    Folder,
};

class VirtualNode {
public:
    VirtualNode(const char* name);
    const char* getName() const;
    virtual FileDescriptor* getDescriptor() = 0;

private:
    const char* _name;
};


class VirtualFolder: VirtualNode {
public:
    VirtualFolder(const char* name);

    virtual FileDescriptor* getDescriptor();
    std::vector<VirtualNode>& getChilds();
private:
    std::vector<VirtualNode*> _childs;
};


class VirtualFileSystem: FileSystem {
public:
    VirtualFileSystem();

    FileDescriptor* getFile(const char* dir);
    std::vector<const char*> getFolderContents();

    // creates a folder in the virtual filesystem with the given path
    void createVirtualFolder(const char* dir);

private:
    VirtualFolder _root;
};

#endif //OLLIOS_GIT_VIRTUALFILESYSTEM_H
