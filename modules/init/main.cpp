#include "file.h"
#include "process.h"
#include <stdio.h>

using namespace fs;

extern "C" void module_load(File* root, const char* argv)
{
    File* file = fs::root->get(argv);

    if (file) {
        std::vector<std::string> args;
        args.push_back(argv);

        Process* p = new Process();
        p->set_arguments(args);
        p->init(file);

        p->open("/sys/vga", 0, 0);
        p->open("/sys/vga", 0, 0);
        p->open("/sys/vga", 0, 0);

        p->start();
    } else {
        printf("Shell not found!\n");
    }
}
