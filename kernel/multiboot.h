//
// Created by Olivier on 28/09/16.
//

#ifndef OLLIOS_GIT_MULTIBOOT_H
#define OLLIOS_GIT_MULTIBOOT_H

#include "types.h"
#include "cdefs.h"

#define MULTIBOOT_MMAP_TYPE_RAM 1

struct __PACKED Multiboot_info_mods {
    void* mod_start;
    void* mod_end;
    char* string;
    u32 reserved;
};

struct __PACKED Multiboot_info_sym {
    u32 tabsize;
    u32 strsize;
    void* addr;
    u32 reserved;
};

struct __PACKED Multiboot_info_elf {
    u32 num;
    u32 size;
    void* addr;
    u32 shndx;
};

union Multiboot_info_sym_or_elf {
    Multiboot_info_elf* elf;
    Multiboot_info_sym* sym;
};

struct __PACKED Multiboot_info_mmap {
    void* base_addr_lower;
    void* base_addr_higher;
    void* length_lower;
    void* length_higher;
    u32 type;
};

struct __PACKED Multiboot_info_drives {
    u32 size;
    // remaining fields are of undefined size so they should be fetched dynamically
};

struct __PACKED Multiboot_info {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    Multiboot_info_mods* mods_addr;
    Multiboot_info_sym_or_elf* syms[3];
    u32 mmap_length;
    Multiboot_info_mmap* mmap_addr;
    u32 drives_length;
    Multiboot_info_drives* drives_addr;
    u32 config_table;
    u32 boot_loader_name;
    u32 apm_table;
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u32 vbe_mode;
    u32 vbe_interface_seg;
    u32 vbe_interface_off;
    u32 vbe_interface_len;
};

#endif //OLLIOS_GIT_MULTIBOOT_H
