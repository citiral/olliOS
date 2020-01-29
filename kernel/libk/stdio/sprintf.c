#include <stdio.h>
#include <stdarg.h>

static int sprintUnsigned(char* target, unsigned int num)
{
    char c[10];
    size_t i = 0;

    //get the characters
    do {
        c[i] = '0' + (num % 10);
        i++;
        num /= 10;
    } while(num > 0);
    size_t k = i;

    //print em out.
    while (k > 0) {
        k--;
        *(target++) = c[k];
    }

    return i;
}

static int sprintHexLower(char* target, unsigned int num)
{
    char c[8];
    size_t i = 0;

    //get the characters
    do {
        if ((num % 0x10) <= 0x9)
            c[i] = '0' + (num % 0x10);
        else
            c[i] = 'a' + (num % 0x10) - 0xA;
        i++;
        num /= 0x10;
    } while(num > 0);
    size_t k = i;

    //print em out.
    while (k > 0) {
        k--;
        *(target++) = c[k];
    }

    return i;
}

static int sprintHexUpper(char* target, unsigned int num)
{
    char c[8];
    size_t i = 0;

    //get the characters
    do {
        if ((num % 0x10) <= 0x9)
            c[i] = '0' + (num % 0x10);
        else
            c[i] = 'A' + (num % 0x10) - 0xA;
        i++;
		num /= 0x10;
    } while(num > 0);
    size_t k = i;

    //print em out.
    while (k > 0) {
        k--;
        *(target++) = c[k];
    }

    return i;
}

static int sprintSigned(char* target, int num)
{
    //print the sign, if needed
    if (num < 0) {
        *(target++) = '-';
        return sprintUnsigned(target, -num) + 1;
    } else {
        return sprintUnsigned(target, num);
    }
}
/*
int sprintf(char* target, const char* format, ...)
{
    //initialisation
    va_list argp;
    va_start(argp, format);
    size_t i = 0;
    int written = 0;
    int temp;
    char c;

    int parsing_cmd = 0;
    unsigned int width;
    

    //for every character
    while((c = format[i]) != '\0')
    {
        //if it is not a %, just print it
        if (c != '%' && !parsing_cmd)
        {
            *(target++) = c;
            written++;
        } else
        {
            //i++;
            //c = format[i];
            switch (c) {
                case '%':
                    width = 0;
                    parsing_cmd = 1;
                    break;
                case 'd':
                case 'i':
                    temp = sprintSigned(target, va_arg(argp, int));
                    written += temp;
                    target += temp;
                    parsing_cmd = 0;
                    break;
                case 'u':
                    temp = sprintUnsigned(target, va_arg(argp, unsigned int));
                    written += temp;
                    target += temp;
                    parsing_cmd = 0;
                    break;
                case 'x':
                    temp = sprintHexLower(target, va_arg(argp, unsigned int));
                    written += temp;
                    target += temp;
                    parsing_cmd = 0;
                    break;
                case 'X':
                    temp = sprintHexUpper(target, va_arg(argp, unsigned int));
                    written += temp;
                    target += temp;
                    parsing_cmd = 0;
                    break;
                case 'c':
                    *(target++) = ((char)va_arg(argp, unsigned int));
                    written += 1;
                    parsing_cmd = 0;
                    break;
                case 's': {
                        char* string = va_arg(argp, char*);

                        while (*string != '\0') {
                            *(target++) = (*string);
                            string++;
                            written += 1;
                        }

                        parsing_cmd = 0;
                    }
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    width = (width*10) + (c - '0');
                    break;
            }
        }
        i++;
    }

    *(target++) = 0;

    va_end(argp);
    return written;
}*/
