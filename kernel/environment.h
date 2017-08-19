#ifndef OLLIOS_GIT_ENVIRONMENT_H
#define OLLIOS_GIT_ENVIRONMENT_H

#include "kstd/unordered_map.h"
#include "kstd/string.h"

class Environment
{
public:
	Environment();
	Environment(const Environment& env);
	~Environment();

	Environment& operator=(const Environment& env);

	// Get an environment variable by name
	const std::string& get(const std::string& name);
	// Set an environment variable
	void set(const std::string& name, const std::string& value);
	// Unset an environment varialbe
	void unset(const std::string& name);
	// Check if an environment varialbe is set
	bool isset(const std::string& name);
private:
	std::unordered_map<std::string, std::string> _vars;
	const std::string _emptyString = "";
};

#endif