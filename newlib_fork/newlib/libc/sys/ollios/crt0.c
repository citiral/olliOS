#include <fcntl.h>
 
extern int main(int argc, char** argv);
extern char** environ;
 
int _start(int argc, char** argv, char** _environ) {
    _environ = environ;
    int ex = main(argc, argv);
    return ex;
}

