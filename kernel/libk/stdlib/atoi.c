//
// Created by citiral on 9/23/16.
//

#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"

int atoi(const char* str)
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

    while (*str != '\0') {
        result = (result * 10) + (str[0] - '0');
        str++;
    }

    return isPos ? result : -result;
}
