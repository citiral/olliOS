//
// Created by Olivier on 09/10/16.
//

#include <string.h>
#include <stddef.h>

size_t strcspn(const char * str1, const char * str2) {
    size_t length = 0;

    // keep looping until str1 is empty
    while (*str1 != '\0') {
        // check if str1 equals anything in str2
        for (int i = 0 ; str2[i] != '\0' ; i++) {
            // if the characters match, return the amount of characters that didn't match
            if (*str1 == *str2) {
                return length;
            }
        }

        // otherwise increase the unmatched characters and advance in the string
        length++;
        str1++;
    }

    // nothing matches so we return the full size of the string
    return length;
}