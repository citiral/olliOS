//
// Created by Olivier on 09/10/16.
//

#include <string.h>
#include <stddef.h>

/*char *strtok_r(char *str, const char *delim, char **saveptr) {
    // if str is null, continue from saveptr
    if (str == nullptr) {
        str = *saveptr;
    }

    // skip all starting characters that are in delim
    while (*str != '\0') {
        bool found = false;
        for (int i = 0 ; delim[i] != '\0' ; i++) {
            if (*str == *delim) {
                found = true;
                break;
            }
        }

        // if a character matches, continue in string, otherwise we break out now
        if (found) {
            str++;
        } else {
            break;
        }
    }


    // skip all starting characters that are in delim
    while (*str != '\0') {
        bool found = false;
        for (int i = 0 ; delim[i] != '\0' ; i++) {
            if (*str == *delim) {
                found = true;
                break;
            }
        }

        // if a character matches, continue in string, otherwise we break out now
        if (found) {
            str++;
        } else {
            break;
        }
    }
}*/