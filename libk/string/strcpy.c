#include <string.h>

char* strcpy(char * destination, const char * source) {
    // copy bytes from source to destination, include null terminator
    size_t i = 0;
    do {
        destination[i] = source[i];
	} while(source[i++] != 0);
	return destination;
}