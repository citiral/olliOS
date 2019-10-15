#include "elf.h"
#include <stdio.h>
#include <string.h>

namespace elf {

#define MAGIC_NUMBER 0x464C457F

static section_header* get_section_header(elf_header* file, u32 header_index)
{
    u8* data = (u8*) file;
    section_header* headers = (section_header*)(data + file->section_header_table_offset);
    return &headers[header_index];
}

static const char* get_section_name(elf_header* file, u32 section_index)
{
    u8* data = (u8*) file;
    section_header* string_table = get_section_header(file, file->section_header_table_string_index);
    section_header* section = get_section_header(file, section_index);

    return (const char*) data + string_table->offset + section->name_offset;
}

static void allocate_nobits(section_header* section)
{
    if (section->size == 0) {
        return;
    }

    if (section->flags & 0x02) {
        void* mem = malloc(section->size);
        memset(mem, 0, section->size);
        section->offset = (u32)mem - (u32)section;
    }
}

static int get_symbol_value(elf_header* elf, section_header* section, u32 symbol_index, SymbolMap& map, u32* out)
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

        *out = entry->offset;
        return 0;
    } else {
        section_header* ndx_header = get_section_header(elf, symbol->section_index);
        *out = (u32)((u8*)elf) + ndx_header->offset + symbol->value;
        return 0;
    }

    return -1;
}

static int relocate_entry(elf* e, elf_header* elf, section_header* section, elf_rel relocation, SymbolMap& map, u32* got)
{
    section_header* target = get_section_header(elf, section->info);
    //printf("relocation target is %s\n", get_section_name(elf, section->info));

    u32 addr = (u32) (((u8*)elf) + target->offset);
    u32* ref = (u32*) (addr + relocation.offset);

    u32 symval;
    //printf("searching symbol with offset %x info %x\n", relocation.offset, relocation.info);
    if (relocation.info >> 8 != 0) {

        if ((u8) relocation.info == (u8) relocation_type::R_386_GOTPC) {
            *ref = (u32)got + *ref - (u32)ref;
            return 0;
        }

        if (get_symbol_value(elf, get_section_header(elf, section->link_index), relocation.info >> 8, map, &symval) != 0) {
            return -1;
        }
        printf("symbol value is %x\n", symval);

        // https://docs.oracle.com/cd/E19683-01/816-7529/chapter6-26/index.html
        switch((u8) relocation.info) {
            case (int) relocation_type::R_386_NONE:
                break;
            case (int) relocation_type::R_386_32:
                *ref = symval + *ref;
                break;
            case (int) relocation_type::R_386_PC32:
            case (int) relocation_type::R_386_PLT32:
                *ref = symval + *ref - (u32)ref;
                break;
            case (int) relocation_type::R_386_GOTOFF:
                *ref = symval + *ref - (u32)got;
                break;
            case (int) relocation_type::R_386_GOT32:
                if (e->_got_count + 1 >= ELF_GOT_SIZE) {
                    printf("GOT overflow\n");
                    return -1;
                }

                
                e->_got_count++;
                e->_GOT[e->_got_count] = symval;
                *ref = e->_got_count*4 + *ref;
                break;
            default:
                printf("Unhandled relocation type\n");
                //return 0;
                return -1;
        }

        return 0;
    }

    return 0;
}

static int relocate_section(elf* e, elf_header* elf, section_header* section, SymbolMap& map, u32* got)
{
    elf_rel* relocation = (elf_rel*) (((u8*)elf) + section->offset);

    for (size_t i = 0 ; i < section->size / sizeof(elf_rel) ; i += 1) {
        printf("relocating.. %d\n", i);
        if (relocate_entry(e, elf, section, relocation[i], map, got) != 0) {
            printf("FAILED relocating.. %d\n", i);
            return -1;
        }
    }

    return 0;
}

elf::elf(u8* data): _header((elf_header*) data), _got_count(0)
{
    memset(_GOT, 0, sizeof(_GOT));
}

int elf::is_valid()
{
    if (_header->magic_number != MAGIC_NUMBER) {
        printf("Magic number does not match.\n");
        return -1;
    }

    if (_header->bit_32_64 != 1) {
        printf("ELF file is not 32-bit\n");
        return -2;
    }

    if (_header->endianness != 1) {
        printf("ELF file is not little endian\n");
        return -3;
    }

    if (_header->type != 0x01) {
        printf("ELF file is not a relocateable file\n");
        return -4;
    }

    if (_header->arch != 0x03) {
        printf("ELF file is not an x86 file\n");
        return -5;
    }

    return 0;
}

int elf::link(SymbolMap& map)
{
    // Allocate NOBITS sections (.bss)
    for (int i = 0 ; i < _header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(_header, i);
        if (section->type == section_type::NOBITS) {
            printf("allocating nobits section: %s\n", get_section_name(_header, i));
            allocate_nobits(section);
        }
    }

    // Handle relocations
    for (int i = 0 ; i < _header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(_header, i);
        if (section->type == section_type::REL) {
            printf("relocating section: %s\n", get_section_name(_header, i));
            if (relocate_section(this, _header, section, map, _GOT) != 0) {
                printf("Failed relocating section %s\n", get_section_name(_header, i));
                return -1;
            }
        }
    } 

    return 0;
}

int elf::get_symbol_value(const char* name, u32* out)
{
    // Look through all symbol tables
    for (int i = 0 ; i < _header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(_header, i);
        if (section->type == section_type::SYMTAB) {

            // Look through all the symbols of this table
            elf_symbol* symbols = (elf_symbol*)(((u8*)_header) + section->offset);
            for (size_t i = 0 ; i < section->size / section->entry_size ; i++) {
                elf_symbol* symbol = symbols + i;

                // get the name of this symbol
                section_header* name_header = get_section_header(_header, section->link_index);
                const char *symbol_name = ((const char*)_header) + name_header->offset + symbol->name_offset;

                // If it matches, return his value
                if (strcmp(name, symbol_name) == 0) {
                    section_header* ndx_header = get_section_header(_header, symbol->section_index);
                    *out = (u32)((u8*)_header) + ndx_header->offset + symbol->value;
                    return 0;
                }
            }
        }
    }
    
    return -1;
}


}
