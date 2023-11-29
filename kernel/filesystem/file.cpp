#include "file.h"
#include "filesystem/registry.h"
#include "filesystem/virtualfolder.h"
#include "filesystem/filelink.h"

using namespace fs;

File* fs::root;

void FileHandle::close() {
    delete this;
}

void fs::init() {
    root = new VirtualFolder("");
    root->bind(new FileLink("..", root));
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
                i++;
            } while (subname[i-1] == child_name[i-1]);
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