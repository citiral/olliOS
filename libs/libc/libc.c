#include "libc.h"
#include <types.h>

char *__env[1] = { 0 };
char **environ = __env;