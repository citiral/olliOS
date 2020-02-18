//
// Created by Olivier on 30/09/16.
//

#include "kernelshell.h"
#include "linker.h"
#include "memory/alloc.h"
#include "memory/virtual.h"
#include "memory/physical.h"
#include "keyboard/keyboard.h"
#include "threading/thread.h"
#include "threading/scheduler.h"
#include "cpu/apic.h"
#include <string.h>
#include "stdio.h"
#include "process.h"
#include "elf/elf.h"

#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
void allocinfo(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	kernelAllocator.printStatistics();
	printf("\n");
}

void allocmerge(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	kernelAllocator.merge();
}
#endif

void cat(KernelShell* shell, std::vector<std::string>* args)
{
	bindings::Binding* bind;
	
	if (args->size() > 1) {
		bind = shell->working_directory->get(args->at(1).c_str());
	} else {
		bind = shell->working_directory;
	}

	if (bind == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

    char buffer[2048];
	size_t total = 0;
	size_t read;
	do {
		read = bind->read(buffer, sizeof(buffer), total);
		for (size_t i = 0 ; i < read ; i++)
			putchar(buffer[i]);
		total += read;
	} while (read > 0);
}

void stat(KernelShell* shell, std::vector<std::string>* args)
{
	bindings::Binding* bind;
	
	if (args->size() > 1) {
		bind = shell->working_directory->get(args->at(1).c_str());
	} else {
		bind = shell->working_directory;
	}

	if (bind == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

	printf("Size: %d\n", bind->get_size());
}

void hex(KernelShell* shell, std::vector<std::string>* args)
{
	bindings::Binding* bind;
	
	if (args->size() > 1) {
		bind = shell->working_directory->get(args->at(1).c_str());
	} else {
		bind = shell->working_directory;
	}

	if (bind == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

    u8 buffer[64];
	size_t total = 0;
	size_t read;
	do {
		read = bind->read(buffer, sizeof(buffer), total);
		for (size_t i = 0 ; i < read ; i++) {
			printf("%.2X", (unsigned int) buffer[i]);;
		}
		total += read;
	} while (read > 0);

	putchar('\n');
}

void ls(KernelShell* shell, std::vector<std::string>* args)
{
	bindings::Binding* bind;
	
	if (args->size() > 1) {
		bind = shell->working_directory->get(args->at(1).c_str());
	} else {
		bind = shell->working_directory;
	}

	if (bind == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

	bind->enumerate([](bindings::Binding*, bindings::Binding* child) {
		printf("%s\n", child->name.c_str());
		return true;
	});
}

void cd(KernelShell* shell, std::vector<std::string>* args)
{
	bindings::Binding* bind;
	
	if (args->size() > 1) {
		bind = shell->working_directory->get(args->at(1).c_str());
	} else {
		bind = bindings::root;
	}


	if (bind == NULL && args->size() > 1) {
		printf("Invalid path: %s\n", args->at(1).c_str());
		return;
	}

	shell->working_directory = bind;
}

void touch(KernelShell* shell, std::vector<std::string>* args)
{
	bindings::Binding* bind = nullptr;
	
	if (args->size() != 2) {		
		printf("Binding name expected\n", args->at(1).c_str());
		return;
	}
	
	bind = shell->working_directory->get(args->at(1).c_str());

	if (bind != nullptr) {
		printf("Bind already exists\n");
		return;
	}

	shell->working_directory->add(new bindings::OwnedBinding(args->at(1)));
}

void help(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(args);
	printf("Possible commands: ");

	for (size_t i = 0; i + 1 < shell->commands().size(); i++)
	{
		printf("%s, ", shell->commands()[i].first);
	}

	if (shell->commands().size() > 0)
	{
		printf("%s", shell->commands().back().first);
	};
	printf("\n");
}

void run(KernelShell* shell, std::vector<std::string>* args)
{
	if (args->size() != 2)
	{
		printf("Usage: load program\n");
		return;
	}

	bindings::Binding* bind = shell->working_directory->get(args->at(1).c_str());
	if (bind == NULL) {
		printf("Invalid path: %s\n");
		return;
	}
	//while (1) {
		Process* p;
		for (int i = 0 ; i < 1000 ; i++) {
			p = new Process();
			p->init(bind);
			p->start();
			//p->wait();
		}
		p->wait();

		//printf("%d", p->status_code);

		//delete p;

		printf("Physical free: %dKB.\n", memory::physicalMemoryManager.countFreePhysicalMemory() * 0x1000 / 1024);
	//}
}

void load(KernelShell* shell, std::vector<std::string>* args)
{
	if (args->size() != 2)
	{
		printf("Usage: load program\n");
		return;
	}

	bindings::Binding* bind = shell->working_directory->get(args->at(1).c_str());
	if (bind == NULL) {
		printf("Invalid path: %s\n");
		return;
	}

	size_t filesize = bind->get_size();
	if (filesize == 0) {
		printf("Filesize is 0.\n");
		return;
	}

	u8* buffer = new u8[filesize];
	size_t total = 0;

	do {
		size_t read = bind->read((void*)(buffer + total), filesize - total, total);
		total += read;
	} while (total != filesize);

	elf::elf* e = new elf::elf(buffer, true);
	if (e == nullptr) {
		printf("Failed allocating elf\n");
		while (1);
	}

	if (e->link_as_kernel_module(*symbolMap) != 0 && 0) {
		printf("failed linking elf\n");
	} else {
		void (*module_load)(bindings::Binding*);
		e->get_symbol_value("module_load", (u32*) &module_load);
		module_load(bindings::root);
	}
}
/*
void set(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	if (args->size() == 2)
	{
		const std::string var = args->at(1);
		if (threading::currentProcess()->env.isset(var))
			printf("%s=%s\n", var.data(), threading::currentProcess()->env.get(var).data());
		else
			printf("%s is not set\n", var.data());
	}
	else if (args->size() == 3)
	{
		const std::string var = args->at(1);
		const std::string val = args->at(2);
		threading::currentProcess()->env.set(var, val);
	}
	delete args;
}

void unset(KernelShell* shell, std::vector<std::string>* args)
{
	UNUSED(shell);
	if (args->size() == 2)
	{
		const std::string var = args->at(1);
		if (threading::currentProcess()->env.isset(var))
			threading::currentProcess()->env.unset(var);
		else
			printf("%s was not set\n", var.data());
	}
	delete args;
}*/

void KernelShell::prompt()
{
	printf("> ");
}

KernelShell::KernelShell(): _commands()
{
    printf("hello world from kernshell\n");
#if __KERNEL_ALLOCATOR == __KERNEL_ALLOCATOR_BUCKET
	_commands.push_back(std::pair<const char *, CommandFunction>("allocinfo", &allocinfo));
	_commands.push_back(std::pair<const char *, CommandFunction>("allocmerge", &allocmerge));
#endif
    _commands.push_back(std::pair<const char*, CommandFunction>("help", &help));
    _commands.push_back(std::pair<const char*, CommandFunction>("ls", &ls));
	_commands.push_back(std::pair<const char*, CommandFunction>("cat", &cat));
	_commands.push_back(std::pair<const char*, CommandFunction>("stat", &stat));
	_commands.push_back(std::pair<const char*, CommandFunction>("hex", &hex));
	_commands.push_back(std::pair<const char*, CommandFunction>("cd", &cd));
	_commands.push_back(std::pair<const char*, CommandFunction>("touch", &touch));
	_commands.push_back(std::pair<const char*, CommandFunction>("load", &load));
	_commands.push_back(std::pair<const char*, CommandFunction>("run", &run));

	working_directory = bindings::root->get("dev/ata0/root/usr/bin");
	prompt();
}

void KernelShell::enter(VirtualKeyEvent input)
{
	// read some virtual key events from the keyboard driver
	//VirtualKeyEvent input[10];
	//size_t read = ((keyboard::KeyboardDriver*) deviceManager.getDevice(DeviceType::Keyboard, 0))->read(input, 10);

    // send them to the input formatter
	_input.handleVirtualKeyEvent(input);

	if (_input.isLineReady())
	{
		std::string line = _input.getNextLine();

        // find the first matching command and call it
        bool notFound = true;

        std::vector<std::string> split = splitCommand(line);

		if (split.size() > 0)
		{
			for (size_t i = 0; i < _commands.size(); i++)
			{
				if (split.at(0) == _commands[i].first)
				{
					notFound = false;
					_commands[i].second(this, &split);
					prompt();
					break;
				}
			}
			// otherwise print a command not found
			if (notFound)
			{
				printf("Unknown command: %s\n", line.data());
			}
		}
    }
}

std::vector<std::pair<const char*, KernelShell::CommandFunction>>& KernelShell::commands() {
	return _commands;
}

std::vector<std::string> KernelShell::splitCommand(std::string cmd)
{
	std::vector<std::string> out;

	// keep looping until the cmd is exhausted
	const char *cmdData = cmd.data();
	while (*cmdData != '\0')
	{
		// first we skip as much whitespace as possible
		while (*cmdData == ' ' || *cmdData == '\t')
			cmdData++;

		// and then we gobble as many characters into the current command as possible
		int length = 0;
		while (cmdData[length] != ' ' && cmdData[length] != '\t' && cmdData[length] != '\0')
			length++;

		if (length == 0)
			break;

		out.push_back(std::string(cmdData, length));

		cmdData += length;
		//TODO "
	}

	return out;
}
