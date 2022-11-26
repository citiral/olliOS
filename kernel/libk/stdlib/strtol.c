//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"
#include "cpu/cpu.h"

long int strtol(const char* str, char** endptr, int base)
{

    int result = 0;

    bool isPos = true;
    
    while (isspace(*str)) {
        str++;
    }

    if (*str == '-') {
        isPos = false;
        str++;
    } else if (*str == '+') {
        isPos = true;
        str++;
    }

    if (base == 10) {
        while (*str >= '0' && *str <= '9') {
            result = (result * base) + (str[0] - '0');
            str++;
        }
    } else if (base == 8) {
        while (*str >= '0' && *str <= '7') {
            result = (result * base) + (str[0] - '0');
            str++;
        }
    } else {
        CPU::panic("Strol called with currently unsupported base");
    }

    if (endptr != nullptr) {
        *endptr = (char*) str;
    }

    return isPos ? result : -result;
}
