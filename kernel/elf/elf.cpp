#include "elf.h"
#include <stdio.h>
#include <string.h>
#include "memory/virtual.h"

namespace elf {

#define MAGIC_NUMBER 0x464C457F

section_header* elf::get_section_header(u32 header_index)
{
    u8* data = (u8*) _header;
    section_header* headers = (section_header*)(data + _header->section_header_table_offset);
    return &headers[header_index];
}

program_header* elf::get_program_header(u32 header_index)
{
    u8* data = (u8*) _header;
    program_header* headers = (program_header*)(data + _header->program_header_table_offset);
    return headers + header_index;
}

const char* elf::get_section_name(u32 section_index)
{
    u8* data = (u8*) _header;
    section_header* string_table = get_section_header(_header->section_header_table_string_index);
    section_header* section = get_section_header(section_index);

    return (const char*) data + string_table->offset + section->name_offset;
}

void elf::allocate_nobits(section_header* section)
{
    if (section->size == 0) {
        return;
    }

    if (section->flags & 0x02) {
        int pages = section->size / (0x1000);
        if (section->size % 0x1000 != 0) {
            pages++;
        }
        void* mem = memory::PageDirectory::current()->bindFirstFreeVirtualPages((void*)(_in_kernel ? 0xC0000000 : 0x40000000), pages);
        memset(mem, 0, section->size);
        section->offset = ((u32)mem - ((u32)_header));
    }
}

const char* elf::get_symbol_name(section_header* section, u32 symbol_index)
{
    elf_symbol* symbols = (elf_symbol*)(((u8*)_header) + section->offset);
    elf_symbol* symbol = symbols + symbol_index;

    section_header* name_header = get_section_header(section->link_index);
    const char *name = ((const char*)_header) + name_header->offset + symbol->name_offset;
    return name;
}

int elf::get_symbol_value(section_header* section, u32 symbol_index, SymbolMap& map, u32* out)
{
    elf_symbol* symbols = (elf_symbol*)(((u8*)_header) + section->offset);
    elf_symbol* symbol = symbols + symbol_index;
    
    if (symbol->section_index == 0) {
        section_header* name_header = get_section_header(section->link_index);
        const char *name = ((const char*)_header) + name_header->offset + symbol->name_offset;
        SymbolMapEntry* entry = map.find_symbol(name);

        if (entry == nullptr) {
            printf("Can't find symbol %s\n", name);
            return -1;
        }

        *out = entry->offset;
        return 0;
    } else {
        section_header* ndx_header = get_section_header(symbol->section_index);
        *out = (u32)_header + ndx_header->offset + symbol->value;

        return 0;
    }

    return -1;
}

int elf::relocate_entry(section_header* section, elf_rel relocation, SymbolMap& map, u32* got)
{
    section_header* target = get_section_header(section->info);
    //printf("relocation target is %s\n", get_section_name(elf, section->info));

    u32 addr = (u32) (((u8*)_header) + target->offset);
    u32* ref = (u32*) (addr + relocation.offset);

    u32 symval;
    //printf("searching symbol with offset %x info %x\n", relocation.offset, relocation.info);
    if (relocation.info >> 8 != 0) {

        if ((u8) relocation.info == (u8) relocation_type::R_386_GOTPC) {
            *ref = (u32)got + *ref - (u32)ref;
            return 0;
        }

        if (get_symbol_value(get_section_header(section->link_index), relocation.info >> 8, map, &symval) != 0) {
            return -1;
        }

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
                if (_got_count + 1 >= ELF_GOT_SIZE) {
                    printf("GOT overflow\n");
                    return -1;
                }

                {
                    const char* name = get_symbol_name(get_section_header(section->link_index), relocation.info >> 8);

                    if (_GotIndex[name] == 0) {
                        _got_count++;
                        _GOT[_got_count] = symval;
                        *ref = _got_count*4 + *ref;
                        _GotIndex[name] = _got_count;
                    } else {
                        *ref = _GotIndex[name]*4 + *ref;
                    }
                }
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

int elf::relocate_section(section_header* section, SymbolMap& map, u32* got)
{
    elf_rel* relocation = (elf_rel*) (((u8*)_header) + section->offset);

    for (size_t i = 0 ; i < section->size / sizeof(elf_rel) ; i += 1) {
        //printf("relocating.. %d\n", i);
        if (relocate_entry(section, relocation[i], map, got) != 0) {
            printf("FAILED relocating.. %d\n", i);
            return -1;
        }
    }

    return 0;
}

elf::elf(u8* data, bool in_kernel): _header((elf_header*) data), _in_kernel(in_kernel), _got_count(0), _GotIndex()
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

int elf::link_as_kernel_module(SymbolMap& map)
{
    // Allocate NOBITS sections (.bss)
    for (int i = 0 ; i < _header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(i);
        if (section->type == section_type::NOBITS) {
            allocate_nobits(section);
        }
    }

    // Handle relocations
    for (int i = 0 ; i < _header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(i);
        if (section->type == section_type::REL) {
            //printf("relocating section: %s\n", get_section_name(_header, i));
            if (relocate_section(section, map, _GOT) != 0) {
                printf("Failed relocating section %s\n", get_section_name(i));
                return -1;
            }
        }
    } 

    return 0;
}

int elf::link_in_userspace()
{
    // Load all sections in memory
    for (int i = 0 ; i < _header->program_header_table_entries ; i++) {
        program_header* header = get_program_header(i);
        if (header->type == program_type::LOAD) {

            // Make sure the VMEM the program header points to exists
            size_t page_count = header->memory_size / 4096;
            if (header->memory_size % 4096 > 0)
                page_count = page_count + 1;

            for (size_t i = 0 ; i < page_count ; i++) {
                memory::PageDirectory::current()->bindVirtualPage(header->virtual_address + i * 4096);
            }

            // And copy the contents of the file into memory
            memcpy(header->virtual_address, ((u8*)_header) + header->offset, header->file_size);

        }
    }

    return 0;
}

int elf::get_symbol_value(const char* name, u32* out)
{
    // Look through all symbol tables
    for (int i = 0 ; i < _header->section_header_table_entries ; i++) {
        section_header* section = get_section_header(i);
        if (section->type == section_type::SYMTAB) {

            // Look through all the symbols of this table
            elf_symbol* symbols = (elf_symbol*)(((u8*)_header) + section->offset);
            for (size_t i = 0 ; i < section->size / section->entry_size ; i++) {
                elf_symbol* symbol = symbols + i;

                // get the name of this symbol
                section_header* name_header = get_section_header(section->link_index);
                const char *symbol_name = ((const char*)_header) + name_header->offset + symbol->name_offset;

                // If it matches, return his value
                if (strcmp(name, symbol_name) == 0) {
                    section_header* ndx_header = get_section_header(symbol->section_index);
                    if (!_in_kernel)
                        *out = symbol->value;
                    else
                        *out = (u32)((u8*)_header) + ndx_header->offset + symbol->value;
                    return 0;
                }
            }
        }
    }
    
    return -1;
}


}
