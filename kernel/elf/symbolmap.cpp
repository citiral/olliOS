#include "symbolmap.h"
#include <stdio.h>

SymbolMap* symbolMap;

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

SymbolMap::SymbolMap(const char* data): symbols_map()
{
    data+=100;
    while (*data != 0) {
        std::string line = "";
        u32 offset = string_to_offset(data);
        u8 type = data[9];

        data += 11;
        while (*data != '\n') {
            line += *data;
            data++;
            //printf("line: %s", line.c_str());
        }
        data++;

        SymbolMapEntry& entry = symbols_map[line];
        printf("adding %s\n", symbols_map[line]);
        entry.type = type;
        entry.offset = offset;
        entry.name = line;
        symbols_list.push_back(entry);
    }
}


SymbolMapEntry* SymbolMap::find_symbol(const char* name)
{
    if (symbols_map.count(name) == 0) {
        return nullptr;
    }

    return &symbols_map[name];
}

SymbolMapEntry* SymbolMap::find_function_name(u32 addr)
{
    for (int i = symbols_list.size() - 1 ; i >= 0 ; i--) {
        if (symbols_list[i].offset <= addr) {
            return &symbols_list[i];
        }
    }

    return NULL;
}