#include <stdio.h>

int main(int argc, char** argv)
{
    for (int i = 0 ; i < 1000 ; i++) {
        malloc(i);
    }
    printf("Hello, world!\n");

    fork();

    return 0;
}
