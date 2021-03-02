#include <stdio.h>
#include <unistd.h>

char buffer[1024];

int main(int argc, char** argv)
{
    while (1)
    {
        int ret = read(0, buffer, sizeof(buffer));
        if (ret == 0) {
            return 0;
        }
        write(1, buffer, ret);
        write(1, buffer, ret);
    }
}

