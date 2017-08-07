#include "fs/virtualfilesystem.h"
#include "cdefs.h"

VirtualFileSystem* vfs;

VirtualFileSystem::VirtualFileSystem() {
    _root.createChildDirectory("fs");
}

VirtualFileSystem::~VirtualFileSystem() {
	
}

void VirtualFileSystem::BindFilesystem(std::string name, FileSystem* fs) {
    _root.getChildDirectory("fs")->fs.push_back(std::pair<std::string, FileSystem*>(name, fs));
}

DirEntry* VirtualFileSystem::getRoot() {
    return new VirtualDirEntry(&_root);
}



VirtualDirEntry::VirtualDirEntry(VirtualDirectory* dir): _vdir(dir), _index(0)  {

}

VirtualDirEntry::~VirtualDirEntry() {

}

bool VirtualDirEntry::valid() {
    return _index < (_vdir->fs.size() + _vdir->directories.size() + _vdir->files.size());
}

bool VirtualDirEntry::advance() {
    _index++;
    return valid();
}

std::string VirtualDirEntry::name() {
    if (_index < _vdir->fs.size()) {
        return _vdir->fs[_index].first;
    } else if (_index - _vdir->fs.size() < _vdir->directories.size()) {
        return _vdir->directories[_index - _vdir->fs.size()]->name;
    }
    return "";
}

DirEntryType VirtualDirEntry::type() {
    return DirEntryType::Folder;
}

DirEntry* VirtualDirEntry::openDir() {
    if (_index < _vdir->fs.size()) {
        return _vdir->fs[_index].second->getRoot();
    } else if (_index - _vdir->fs.size() < _vdir->directories.size()) {
        return new VirtualDirEntry(_vdir->directories[_index - _vdir->fs.size()]);
    }
    return nullptr;
}

Stream* VirtualDirEntry::openFile() {
    return nullptr;
}



VirtualDirectory* VirtualDirectory::createChildDirectory(std::string name) {
    VirtualDirectory* ndir = new VirtualDirectory();
    ndir->name = name;
    directories.push_back(ndir);
    return ndir;
}

VirtualDirectory* VirtualDirectory::getChildDirectory(const char* name) {
    for (int i = 0 ; i < directories.size() ; i++) {
        if (directories[i]->name == name) {
            return directories[i];
        }
    }
    return nullptr;
}