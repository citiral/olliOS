#include "shell.h"
#include "keyboard.h"
#include "inputbuffer.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    printf("Hello world from %s.\n", argv[0]);

    if (argc < 2) {
        printf("Error, init called with not enough parameters.\n");
        exit(-1);
    }

    Shell shell = spawn_shell(argv[1]);

    FILE* keyb = fopen("/sys/keyboard", "rb+");
    if (keyb == NULL) {
        printf("Failed opening keyboard\n");
    }
    int err = setvbuf(keyb, NULL, _IONBF, 0);
    if (err != 0) {
        printf("Failed setbuf: %d\n", err);
    }

    InputBuffer b = create_input_buffer(shell.input);

    while (1) {
        VirtualKeyEvent key;
        fread(&key, sizeof(VirtualKeyEvent), 1, keyb);
        process_event(&b, key);
    }

    fclose(keyb);
}