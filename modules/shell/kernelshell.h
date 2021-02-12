//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_KERNELSHELL_H
#define OLLIOS_GIT_KERNELSHELL_H

#include "inputformatter.h"
#include "kstd/vector.h"
#include "kstd/utility.h"
#include "kstd/string.h"
#include "file.h"

class KernelShell {
using CommandFunction = void (*)(KernelShell* shell, std::vector<std::string>* args);
public:
    KernelShell();

    void prompt();
    void enter(VirtualKeyEvent input); 
    void runCommand(std::string command);   

    std::vector<std::pair<const char*, CommandFunction>>& commands();
    fs::File* working_directory;

private:
    std::vector<std::string> splitCommand(std::string cmd);

    InputFormatter _input;
	std::vector<std::pair<const char*, CommandFunction>> _commands;
};

#endif //OLLIOS_GIT_KERNELSHELL_H
