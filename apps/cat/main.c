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

    FILE* test = fopen("/dev/ata0/root/include/ar.h", "rb+");
    int read = 1;
    int written = 0;

    do {
        read = fread(buffer, 1, sizeof(buffer), test);
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