#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_value(FILE* file)
{
    fseek(file, 0, SEEK_SET);
    char buffer[64];
    fread(buffer, 1, sizeof(buffer), file);
    return atoi(buffer);
}

void set_value(FILE* file, int value)
{
    fseek(file, 0, SEEK_SET);
    char buffer[64];
    sprintf(buffer, "%d", value);
    fputs(buffer, file);
}

FILE* open_file(const char* path)
{
    FILE* file = fopen(path, "rb+");
    setbuf(file, NULL);
    return file;
}

int main(int argc, char** argv)
{
    FILE* height = open_file("/sys/vga/height");
    FILE* row = open_file("/sys/vga/row");

    int h = get_value(height);

    for (int i = 0 ; i < h ; i++) {
        putchar('\n');
    }
    set_value(row, 0);
}