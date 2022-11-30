#include <string.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc > 1) {
        printf("%s", argv[1]);
    }
    for (int i = 2 ; i < argc ; i++) {
        printf(" %s", argv[i]);
    }

    printf("\n");

    return 0;
}
