#include "filesystem/file.h"
#include "process.h"
#include <stdio.h>

using namespace fs;

std::string get_path_for(std::string file)
{
    for (int i = file.length() - 1 ; i >= 0 ; i--) {
        if (file[i] == '/') {
            return file.substr(0, i);
        }
    }
    return file;
}

extern "C" void module_load(File* root, const char* argv)
{
    std::string arg(argv);

    int wdlength = 0;
    while (argv[wdlength] != '\0' && argv[wdlength] != ' ') {
        wdlength++;
    }

    std::string filepath = arg.substr(0, wdlength);
    std::string wd = get_path_for(filepath);
    std::string args = argv + wdlength + 1;

    File* file = fs::root->get(filepath.c_str());
    if (file) {
        std::vector<std::string> arglist;
        arglist.push_back(filepath);
        arglist.push_back(args);

        Process* p = new Process();
        p->set_arguments(arglist);
        p->init(file, wd);

        p->open("/dev/vga", 0, 0);
        p->open("/dev/vga", 0, 0);
        p->open("/dev/vga", 0, 0);

        p->start();
    } else {
        printf("Shell not found!\n");
    }
}
