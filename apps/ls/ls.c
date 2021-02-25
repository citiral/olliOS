#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

void print_listing(char* folder)
{
    DIR* d;
    struct dirent* de;

    d = opendir(folder);

    int i = 0;
    while ((de = readdir(d)) != NULL) {
        printf("%s\t", de->d_name);
    }
    printf("\n");

    closedir(d);
}

int main(int argc, char** argv)
{
    DIR* d;
    struct dirent* de;

    if (argc == 1) {
        print_listing(".");
    } else if (argc == 2) {
        print_listing(argv[1]);
    } else {
        for (int i = 1 ; i < argc ; i++) {
            printf("%s:\n", argv[i]);
            print_listing(argv[i]);
        }
    }

    return 0;
}