#include "file.h"
#include "multiboot.h"
#include "types.h"
#include "tar.h"
#include "string.h"
#include "stdlib.h"
#include "file.h"
#include "virtualfile.h"

star_header* get_initfs_tar_file()
{
    multiboot_module_t *mod = (multiboot_module_t*) multiboot::instance->mods_addr;
    return (star_header*) mod[1].mod_start;
}

int split_filename(char* name) {
    int parts = 1;
    while (*name != '\0') {
        if (*name == '/') {
            *name =- '\0';
            parts++;
        }
        name++;
    }
    return parts;
}

fs::File* make_tarfs_file(fs::File* base, int parts, char* path) {
    for (int i = 0 ; i < parts - 1 ; i++) {
        auto next = base->get(path);
        if (next == nullptr) {
            next = new fs::VirtualFolder(path);
            base->bind(next);
        }
        base = next;
        path += strlen(path) + 1;
    }

    auto file = new fs::VirtualFile(path);
    base->bind(file);
    return file;
}

extern "C" void module_load(fs::File* root, const char* argv)
{
    UNUSED(root);
    UNUSED(argv);

    star_header* tar_file = get_initfs_tar_file();

    auto base = new fs::VirtualFolder("initfs");
    root->bind(base);

    while (strcmp(tar_file->magic, OLDGNU_MAGIC) == 0) {
        int size = strtol(tar_file->size, nullptr, 8);
        int offset = (size + sizeof(star_header) - 1) / sizeof(star_header);

        if (size > 0) {
            std::string filename = tar_file->name;
            int parts = split_filename(tar_file->name);
            auto file = make_tarfs_file(base, parts, tar_file->name);
            auto handle = file->open();
            handle->write(tar_file + 1, size, 0);
            handle->close();
        }

        tar_file += offset + 1;
    }
}