#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int vga;

void print(char* data)
{
    write(vga, data, strlen(data)); 
}

int main(int argc, char **argv)
{
    char buffer[1024];

    if (argc != 2) {
        fprintf(stderr, "Error, usage: %s target\n", argv[0]);
        return -1;
    }

    FILE* test = fopen(argv[1], "rb+");
    int read = 1;
    int written = 0;

    printf("openened.\n");

    do {
        printf("readning test: %X\n", *test);
        read = fread(buffer, 1, sizeof(buffer), test);
        printf("DOEN\n");
        written = 0;
        if (read > 0) {
            while (written + 1 < read) {
                written += fwrite(buffer + written, 1, read - written, stdout);
            }
        }
    } while (read > 0);

    fclose(test);

    return 0;
}