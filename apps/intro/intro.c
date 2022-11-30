#include <stdio.h>
#include <unistd.h>
#include <string.h>

int get_value(FILE* file)
{
    fseek(file, 0, SEEK_SET);
    char buffer[64];
    fread(buffer, 1, sizeof(buffer), file);
    return atoi(buffer);
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
    FILE* fb = open_file("/sys/vga/framebuffer");

    int width = get_value(fWidth);
    int height = get_value(fHeight);

    fclose(fWidth);
    fclose(fHeight);

    char* framebuffer = malloc(width * height);
    free(framebuffer);
    
    for (int i = 0 ; i < 10 ; i++) {
        memset(framebuffer, '0'+i, width*height);
        fwrite(framebuffer, 1, width*height, fb);
        fseek(fb, 0, SEEK_SET);
        usleep(1000000);
    }

    fclose(fb);

    getchar();

    return 0;
}