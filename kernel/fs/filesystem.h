//
// Created by Olivier on 01/10/16.
//

#ifndef OLLIOS_GIT_FILESYSTEM_H
#define OLLIOS_GIT_FILESYSTEM_H

#include "../streams/stream.h"
#include <vector>

/*
 * Abstract filesystem. This provides an API that can be implemented by physical and virtual filesystems.
 */
class FileSystem {
    /*
     * Returns the list of files contained in the given directory
     */
    std::vector<File> readDirectory(const char* directory) = 0;

    /*
     * Returns the list of files contained in the given file (if the file is a directory)
     */
    std::vector<File> readDirectory(File* file) = 0;

    /*
     * returns the root directory of the filesystem
     */
    File* getRoot() = 0;
};

enum class FileType: u8 {
    FOLDER = 0,
    FILE = 1,
};

/*
 * Abstract file interface. This provides an API that can be implemented by physical and virtual files.
 */
class File: public Stream {
    //returns the type of the file (device or folder)
    virtual FileType getDeviceType() const = 0;

    //returns the name of the file.
    virtual const char* getFileName() const = 0;
};

#endif //OLLIOS_GIT_FILESYSTEM_H
