#ifndef __ELF_H
#define __ELF_H

#include "types.h"
#include "symbolmap.h"
#include "kstd/unordered_map.h"

#define ELF_GOT_SIZE 1024

namespace elf {

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
    R_386_NONE = 0,
    R_386_32 = 1,
    R_386_PC32 = 2,
    R_386_GOT32 = 3,
    R_386_PLT32 = 4,
    R_386_GOTOFF = 9,
    R_386_GOTPC = 10,
    //R_386_GOT32X = 43,
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


class elf {
public:
    elf(u8* data);

    int is_valid();
    int link(SymbolMap& map);
    int get_symbol_value(const char* name, u32* out);
    
    elf_header* _header;
    u32 _GOT[ELF_GOT_SIZE];
    u32 _got_count;
    std::unordered_map<const char*, u32> _GotIndex;
};

void dump_elf(u8* file, SymbolMap& map);

}

#endif
