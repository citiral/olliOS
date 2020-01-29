#include <stdio.h>
#include <stdarg.h>

typedef enum printf_mode {
    PRINTF_STDOUT,
    PRINTF_STRING,
}  printf_mode;

typedef struct {
    printf_mode mode;
    int width;
    int precision;
    int has_precision;
    int parsing_cmd;
    char* target;
    int written;
} printf_config;

static void write(printf_config* p,  char c)
{
    switch (p->mode) {
        case PRINTF_STDOUT:
            putchar(c);
            break;
        case PRINTF_STRING:
            *p->target = c;
            p->target++;
            break;
    }
    p->written++;
}

static void printUnsigned(printf_config* p, unsigned int num)
{
    char c[10];
    size_t i = 0;

    //get the characters
    do {
        c[i] = '0' + (num % 10);
        i++;
        num /= 10;
    } while(num > 0);

    //print em out.
    for (int k = i ; k < p->precision ; k++) {
        write(p, '0');
    }

    while (i > 0) {
        i--;
        write(p, c[i]);
    }
}

static void printSigned(printf_config* p, int num)
{
    //print the sign, if needed
    if (num < 0) {
        write(p, '-');
        printUnsigned(p, -num);
    } else {
        printUnsigned(p, num);
    }
}

static void printHexLower(printf_config* p, unsigned int num)
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

    //print em out.
    for (int k = i ; k < p->precision ; k++) {
        write(p, '0');
    }
    while (i > 0) {
        i--;
        write(p, c[i]);
    }
}

static void printHexUpper(printf_config* p, unsigned int num)
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

    //print em out.
    for (int k = i ; k < p->precision ; k++) {
        write(p, '0');
    }
    while (i > 0) {
        i--;
        write(p, c[i]);
    }
}

int _printf(printf_config* p, const char* format, va_list argp)
{
    //initialisation
    size_t i = 0;
    char c;
    p->written = 0;
    p->parsing_cmd = 0;
    
    //for every character
    while((c = format[i]) != '\0')
    {
        //if it is not a %, just print it
        if (c == '%' || p->parsing_cmd)
        {
            switch (c) {
                case '%':
                    p->width = -1;
                    p->precision = -1;
                    p->has_precision = 0;
                    p->parsing_cmd = 1;
                    break;
                case 'd':
                case 'i':
                    printSigned(p, va_arg(argp, int));
                    p->parsing_cmd = 0;
                    break;
                case 'u':
                    printUnsigned(p, va_arg(argp, unsigned int));
                    p->parsing_cmd = 0;
                    break;
                case 'x':
                    printHexLower(p, va_arg(argp, unsigned int));
                    p->parsing_cmd = 0;
                    break;
                case 'X':
                    printHexUpper(p, va_arg(argp, unsigned int));
                    p->parsing_cmd = 0;
                    break;
                case 'c':
                    write(p, (char)va_arg(argp, unsigned int));
                    p->parsing_cmd = 0;
                    break;
                case 's': {
                        char* string = va_arg(argp, char*);

                        while (*string != '\0') {
                            write(p, *string);
                            string++;
                        }

                        p->parsing_cmd = 0;
                    }
                    break;
                case '.':
                    p->has_precision = 1;
                    p->precision = 0;
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
                    if (p->has_precision) {
                        p->precision = (p->precision*10) + (c - '0');
                    } else {
                        p->width = (p->width*10) + (c - '0');
                    }
                    break; 
            }
        } else {            
            write(p, c);
        }
        
        i++;
    }

    return p->written;
}

int printf(const char* format, ...)
{
    printf_config p;
    p.mode = PRINTF_STDOUT;
    
    va_list argp;
    va_start(argp, format);
    int value = _printf(&p, format, argp);    
    va_end(argp);

    return  value;
}

int sprintf(char* target, const char* format, ...)
{
    printf_config p;
    p.mode = PRINTF_STRING;
    p.target = target;
    
    va_list argp;
    va_start(argp, format);
    int value = _printf(&p, format, argp);    
    va_end(argp);

    *p.target = NULL;

    return  value;
}
