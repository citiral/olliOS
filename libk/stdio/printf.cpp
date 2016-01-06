#include <stdio.h>
#include <stdarg.h>

int printUnsigned(unsigned int num)
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
        putchar(c[k]);
    }

    return k;
}

int printSigned(int num)
{
    //print the sign, if needed
    if (num < 0) {
        putchar('-');
        return printUnsigned(-num) + 1;
    } else {
        return printUnsigned(num);
    }
}

int printf(const char* format, ...)
{
    //initialisation
    va_list argp;
    va_start(argp, format);
    size_t i = 0;
    int written = 0;
    char c;

    //for every character
    while((c = format[i]) != '\0')
    {
        //if it is not a %, just print it
        if (c != '%')
        {
            putchar(c);
            written++;
        } else
        {
            i++;
            c = format[i];
            switch (c) {
                case 'd':
                case 'i':
                    written += printSigned(va_arg(argp, int));
                    break;
                case 'u':
                    written += printUnsigned(va_arg(argp, unsigned int));
                    break;
            }
        }
        i++;
    }

    va_end(argp);
    return written;
}
