//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_KERNELSHELL_H
#define OLLIOS_GIT_KERNELSHELL_H

#include "util/inputformatter.h"
#include "kstd/vector.h"
#include "kstd/utility.h"
#include "kstd/string.h"
#include "environment.h"

class KernelShell {
public:
    KernelShell();
    void enter();

private:
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
    void allocinfo(Environment& env, std::vector<std::string> args);
	void allocmerge(Environment& env, std::vector<std::string> args);
#endif
    void devicesinfo(Environment& env, std::vector<std::string> args);
    void ls(Environment& env, std::vector<std::string> args);
    void cat(Environment& env, std::vector<std::string> args);
	void help(Environment& env, std::vector<std::string> args);
	void set(Environment& env, std::vector<std::string> args);
	void unset(Environment& env, std::vector<std::string> args);
    std::vector<std::string> splitCommand(std::string cmd);

    using CommandFunction = void (KernelShell::*)(Environment& env, std::vector<std::string> args);
    InputFormatter _input;
	std::vector<std::pair<const char*, CommandFunction>> _commands;
	Environment _env;
};

#endif //OLLIOS_GIT_KERNELSHELL_H
