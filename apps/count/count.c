#include <stdio.h>
#include <unistd.h>

void count(unsigned int limit)
{
    unsigned int i = 0;
    while (1) {
        sleep(1);
        printf("%d\n", i+1);
        i++;
        if (i == limit) {
            break;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc == 1) {
        count(0);
    } else {
        int l = 0;
        sscanf(argv[1], "%u", &l);
        count(l);
    }
}