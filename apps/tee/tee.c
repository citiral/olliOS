#include <stdio.h>
#include <unistd.h>

char buffer[1024];

int main(int argc, char** argv)
{
    while (1)
    {
        int file = 1;
        if (argc == 2) {
            file = open(argv[1]);
        }

        int ret = read(0, buffer, sizeof(buffer));
        if (ret == 0) {
            if (file != 1) {
                close(file);
            }
            return 0;
        }
        write(1, buffer, ret);
        write(file, buffer, ret);
    }
}

