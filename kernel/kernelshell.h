//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_KERNELSHELL_H
#define OLLIOS_GIT_KERNELSHELL_H

#include "util/inputformatter.h"
#include "kstd/vector.h"
#include "kstd/utility.h"

class KernelShell {
public:
    KernelShell();
    void enter();

private:
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
    void allocinfo(const char* cmd);
    void allocmerge(const char* cmd);
    void devicesinfo(const char* cmd);
    void ls(const char* cmd);
#endif
    void help(const char* cmd);

    using CommandFunction = void (KernelShell::*)(const char* cmd);
    InputFormatter _input;
    std::vector<std::pair<const char*, CommandFunction>> _commands;
};

#endif //OLLIOS_GIT_KERNELSHELL_H
