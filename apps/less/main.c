#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <_ansi.h>
#include <unistd.h>

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
    FILE* fWidth = open_file("/sys/vga/width");
    FILE* fHeight = open_file("/sys/vga/height");
    FILE* fRow = open_file("/sys/vga/row");
    FILE* fCol = open_file("/sys/vga/column");

    int inp = open("/dev/stdin", O_RDONLY );

    int width = get_value(fWidth);
    int height = get_value(fHeight);

    int lineCount = 0;
    
    for (int i = 0 ; i < height ; i++) {
        putchar('\n');
    }
    set_value(fRow, 0);

    char buffer[width+1];

    while (fgets(buffer, width+1, stdin) != NULL) {
        if (ferror(stdin)) {
            break;
        }

        printf("%s", buffer);
        
        lineCount++;
        if (lineCount >= height) {
            set_value(fRow, height - 2);
            set_value(fCol, 0);
            char b[2];
            read(inp, b, 2);
        }
    }

    fclose(fWidth);
    fclose(fHeight);
    close(inp);
}