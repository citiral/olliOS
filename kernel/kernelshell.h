//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_KERNELSHELL_H
#define OLLIOS_GIT_KERNELSHELL_H

#include "util/inputformatter.h"
#include "kstd/vector.h"
#include "kstd/utility.h"
#include "kstd/string.h"

class KernelShell {
public:
    KernelShell();
    void enter();

private:
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
    void allocinfo(std::vector<std::string> args);
    void allocmerge(std::vector<std::string> args);
    void devicesinfo(std::vector<std::string> args);
    void ls(std::vector<std::string> args);
#endif
    void help(std::vector<std::string> args);
    std::vector<std::string> splitCommand(std::string cmd);

    using CommandFunction = void (KernelShell::*)(std::vector<std::string> args);
    InputFormatter _input;
	std::vector<std::pair<const char*, CommandFunction>> _commands;
};

#endif //OLLIOS_GIT_KERNELSHELL_H
