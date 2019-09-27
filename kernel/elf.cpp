#include "elf.h"
#include <stdio.h>

namespace elf {

#define MAGIC_NUMBER 0x464C457F

struct program_header {
    u32 type;
    u32 offset;
    void* virtual_address;
    void* physical_address;
    u32 file_size;
    u32 memory_size;
    u32 flags;
    u32 alignment;
} __attribute__((packed));

struct section_header {
    u32 name_offset;
    u32 type;
    u32 flags;
    void* virtual_address;
    u32 offset;
    u32 size;
    u32 link_index;
    u32 info;
    u32 allignment;
    u32 fixed_size_size;
} __attribute__((packed));

struct elf_header {
    u32 magic_number;
    u8 bit_32_64;
    u8 endianness;
    u8 version;
    u8 os_abi;
    u8 abi_version;
    u8 __pad0[7];
    u16 type;
    u16 arch;
    u32 e_version;
    void* entry_point;
    u32 program_header_table_offset;
    u32 section_header_table_offset;
    u32 flags;
    u16 header_size;
    u16 program_header_table_entry_size;
    u16 program_header_table_entries;
    u16 section_header_table_entry_size;
    u16 section_header_table_entries;
    u16 section_header_table_string_index;
} __attribute__((packed));

program_header* get_program_header(elf_header* file, u32 header_index)
{
    u8* data = (u8*) file;
    program_header* headers = (program_header*) data + file->program_header_table_offset;
    return &headers[header_index];
}

section_header* get_section_header(elf_header* file, u32 header_index)
{
    u8* data = (u8*) file;
    section_header* headers = (section_header*)(data + file->section_header_table_offset);
    return &headers[header_index];
}

const char* get_section_name(elf_header* file, u32 section_index)
{
    u8* data = (u8*) file;
    section_header* string_table = get_section_header(file, file->section_header_table_string_index);
    section_header* section = get_section_header(file, section_index);

    return (const char*) data + string_table->offset + section->name_offset;
}

void dump_elf(u8* file)
{
    elf_header* header = (elf_header*) file;
    for (int i = 0 ; i < 4 ; i++) {
        putchar(file[i]);
    }

    if (header->magic_number != MAGIC_NUMBER) {
        printf("Magic number does not match.\n");
        return;
    }

    if (header->bit_32_64 != 1) {
        printf("ELF file is not 32-bit\n");
        return;
    }

    if (header->endianness != 1) {
        printf("ELF file is not little endian\n");
        return;
    }

    if (header->type != 0x01) {
        printf("ELF file is not a relocateable file\n");
        return;
    }

    if (header->arch != 0x03) {
        printf("ELF file is not an x86 file\n");
        return;
    }

    printf("program headers: %d\n", header->program_header_table_entries);
    printf("section headers: %d\n", header->section_header_table_entries);
    printf("string section header %d\n", header->section_header_table_string_index);

    const char* data = (const char *)header;
    section_header* str_header = get_section_header(header, header->section_header_table_string_index);

    for (int i = 0 ; i < str_header->size ; i++) {
       putchar(data[i + str_header->offset]);
    } 

    for (int i = 0 ; i < header->section_header_table_entries ; i++) {
        printf("section: %s\n", get_section_name(header, i));
    }
}

}

