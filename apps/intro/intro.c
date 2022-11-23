#include <stdio.h>

int main(int argc, char** argv)
{
    char name[128];
    printf("Hello, and welcome to Ollios.\nTo continue, please enter your name:\n");
    gets(name);
    printf("Pleased to meet you, %s.\n", name);

    return 0;
}