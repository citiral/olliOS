#pragma once

#include <stdbool.h>

int run_command(int pipein, char** arguments, bool blocking, bool pipeOutput);