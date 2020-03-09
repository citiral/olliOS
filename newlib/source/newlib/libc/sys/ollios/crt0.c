#include <fcntl.h>
 
extern void exit(int code);
extern int main(int argc, char** argv);
extern char** environ;
 
void _start(int argc, char** argv, char** _environ) {
    _environ = environ;
    int ex = main(argc, argv);
    exit(ex);
}

