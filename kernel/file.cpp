#include "file.h"
#include "filesystem.h"
#include "virtualfile.h"

using namespace fs;

File* fs::root;

void FileHandle::close() {
    delete this;
}

void fs::init() {
    root = new VirtualFolder("");
    registry = new Registry();
}

File* _get(File* f, const char* name)
{
    const char* subname = name;

    // Skip leading slash
    while (subname[0] == '/') {
        subname++;

        // True if the path ends with a '/', in which case we are the targetted file
        if (subname[0] == 0) {
            return f;
        }
    }

    // Handle '.' and '..'
    if (subname[0] == '.') {
        if (subname[1] == '/') {
            return f->get(subname+2);
        } else if (subname[1] == 0) {
            return f;
        } else if (subname[1] == '.') {
            if (subname[2] == '/') {
                /*if (_parent) {
                    return NULL;//_parent->get(subname+3);
                } else {*/
                    return NULL;
                //}
            } else if (subname[2] == 0) {
                return NULL;//_parent;
            }
        }
    }

    FileHandle *handle = f->open();
    if (handle) {
        File* child;

        while ((child = handle->next_child()) != NULL) {
            int i = 0;
            const char* child_name = child->get_name();
            do {
                // End of section in path has been reached
                if (child_name[i] == 0) {
                    if (subname[i] == 0) {
                        handle->close();
                        return child;
                    } else if (subname[i] == '/') {
                        handle->close();
                        return _get(child, subname+i);
                    } else {
                        break;
                    }
                }
            } while (subname[i] == child_name[i++]);
        }

        handle->close();
    }

    return NULL;
}

File* File::get(const char* name)
{
    if (name[0] == '/' && this != root) {
        return _get(root, name);
    } else {
        return _get(this, name);
    }
}