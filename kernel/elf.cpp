#include "elf.h"
#include <stdio.h>
#include <string.h>

namespace elf {

#define MAGIC_NUMBER 0x464C457F

enum class section_type: u32 {
    SNULL = 0,
    PROGBITS = 1,
    SYMTAB = 2,
    STRTAB = 3,
    RELA =4,
    HASH = 5,
    DYNAMIC = 6,
    NOTE = 7,
    NOBITS = 8,
    REL = 9,
    SHLIB = 0xA,
    DYNSYM = 0xB,
    INIT_ARRAY = 0xE,
    FINI_ARRAY = 0xF,
    PREINIT_ARRAY = 0x10,
    GROUP = 0x11,
    SYMTAB_SHNDX = 0x12,
    NUM = 0x13,
};

enum class relocation_type: u32 {
    R_386_PC32 = 2,
    R_386_PLT32 = 4,
};

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
    section_type type;
    u32 flags;
    void* virtual_address;
    u32 offset;
    u32 size;
    u32 link_index;
    u32 info;
    u32 allignment;
    u32 entry_size;
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

struct elf_symbol {
    u32 name_offset;
    u32 value;
    u32 size;
    u8 flags;
    u8 other;
    u16 section_index;
} __attribute__((packed));

struct elf_rel {
    u32 offset;
    u32 info;
} __attribute__((packed));

struct elf_rela {
    u32 offset;
    u32 info;
    i32 addend;
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

void allocate_nobits(section_header* section)
{
    if (section->size == 0) {
        return;
    }

    printf("TODO: implement nobits allocation");
}

u32 get_symbol_value(elf_header* elf, section_header* section, u32 symbol_index, SymbolMap& map)
{
    elf_symbol* symbols = (elf_symbol*)(((u8*)elf) + section->offset);
    elf_symbol* symbol = symbols + symbol_index;
    
    if (symbol->section_index == 0) {
        section_header* name_header = get_section_header(elf, section->link_index);
        const char *name = ((const char*)elf) + name_header->offset + symbol->name_offset;
        SymbolMapEntry* entry = map.find_symbol(name);

        if (entry == nullptr) {
            printf("Can't find symbol %s\n", name);
        } else {
            printf("Found symbol %s at %X\n", name, entry->offset);
        }

        return entry->offset - 0xC0000000;
    } else {
        section_header* ndx_header = get_section_header(elf, symbol->section_index);
        return (u32)((u8*)elf) + ndx_header->offset + symbol->value;
    }

    return 0;
}

u32 find_symbol_value(elf_header* elf, section_header* section, const char *symbol_name, SymbolMap& map)
{
    elf_symbol* symbols = (elf_symbol*)(((u8*)elf) + section->offset);

    for (size_t i = 0 ; i < section->size / section->entry_size ; i++) {
        elf_symbol* symbol = symbols + i;

        section_header* name_header = get_section_header(elf, section->link_index);
        const char *name = ((const char*)elf) + name_header->offset + symbol->name_offset;

        printf("testing symbol %s\n", name);

        if (strcmp(name, symbol_name) == 0) {
            printf("found symbol %s\n", symbol_name);
            return get_symbol_value(elf, section, i, map);
        }
    }

    return 0;
}

void relocate_entry(elf_header* elf, section_header* section, elf_rel relocation, SymbolMap& map)
{
    section_header* target = get_section_header(elf, section->info);
    //printf("relocation target is %s\n", get_section_name(elf, section->info));

    u32 addr = (u32) (((u8*)elf) + target->offset);
    u32* ref = (u32*) (addr + relocation.offset);

    u32 symval;
    //printf("searching symbol with offset %x info %x\n", relocation.offset, relocation.info);
    if (relocation.info >> 8 != 0) {
        symval = get_symbol_value(elf, get_section_header(elf, section->link_index), relocation.info >> 8, map);
        printf("symbol value is %x\n", symval);

        switch((u8) relocation.info) {
            case (int) relocation_type::R_386_PC32:
            case (int) relocation_type::R_386_PLT32:
                *ref =  symval + *ref - (u32)ref;
            default:
                printf("Unhandled relocation type\n");
        }
    }
}

void relocate_section(elf_header* elf, section_header* section, SymbolMap& map)
{
    elf_rel* relocation = (elf_rel*) (((u8*)elf) + section->offset);

    for (size_t i = 0 ; i < section->size / sizeof(elf_rel) ; i += 1) {
        printf("relocating..\n");
        relocate_entry(elf, section, relocation[i], map);
    }
}

void dump_elf(u8* file, SymbolMap& map)
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

    // Allocate NOBITS sections (.bss)
    for (int i = 0 ; i < header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(header, i);
        if (section->type == section_type::NOBITS) {
            printf("allocating nobits section: %s\n", get_section_name(header, i));
            allocate_nobits(section);
        }
    }

    for (int i = 0 ; i < header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(header, i);
        if (section->type == section_type::REL) {
            printf("relocating section: %s\n", get_section_name(header, i));
            relocate_section(header, section, map);
        }
    } 

    for (int i = 0 ; i < header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(header, i);
        if (section->type == section_type::SYMTAB) {
            printf("found symbol table\n");
            u32 module_load = find_symbol_value(header, section, "module_load", map) + 0xC0000000;
            u8* data = (u8*)module_load;
            printf("module load is %x\n", module_load);

            int (*entry)(void) = (int(*)())module_load;
            printf("result is %d\n", entry());
        }
    }  
}

}


extern "C" void testprint(const char *s) {
    printf("string adr is %x", s);
    printf(s - 0xC0000000);
}
