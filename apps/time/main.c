#include <stdio.h>
#include <time.h>
#include <sys/syscalls.h>


int main(int argc, char** argv)
{
    FILE* screen = fopen("/sys/vga/framebuffer", "rb+");
    setbuf(screen, NULL);

    while (1) {
        cmos_time_t t = {};
        int res = _get_cmos_utc(&t);
        if (res == -1) {
            printf("Error getting time.\n");
            return -1;
        } else {
            char outp[127];
            sprintf(outp, "%d/%d/%d %2d:%02d:%02d", (int) t.day, (int) t.month, (int) t.year, (int) t.hours, (int) t.minutes, (int) t.seconds);
            fseek(screen, 0, SEEK_SET);
            fwrite(outp, 1, strlen(outp), screen);
        }
        sleep(1);
    }

    fclose(screen);
}
