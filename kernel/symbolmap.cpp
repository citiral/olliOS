#include "symbolmap.h"
#include <stdio.h>

u32 string_to_offset(const char* str)
{
    u32 offset = 0;

    while (*str != 0 && *str != ' ') {
        offset *= 16;

        char c = *str;
        if (c >= '0' && c <= '9') {
            offset += c - '0';
        } else if (c >= 'a' && c <= 'f') {
            offset += c - 'a' + 10;
        } else {
            offset += c - 'A' + 10;
        }
        
        str++;
    }

    return offset;
}

SymbolMap::SymbolMap(const char* data): symbols()
{

    while (*data != 0) {
        std::string line = "";
        u32 offset = string_to_offset(data);
        u8 type = data[9];

        data += 11;
        while (*data != '\n') {
            line += *data;
            data++;
        }
        data++;

        SymbolMapEntry& entry = symbols[line];
        entry.type = type;
        entry.offset = offset;
    }
}


SymbolMapEntry* SymbolMap::find_symbol(const char* name)
{
    if (symbols.count(name) == 0) {
        return nullptr;
    }

    return &symbols[name];
}
