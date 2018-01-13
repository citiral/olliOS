#include "environment.h"

Environment::Environment(): _vars()
{
	//_vars = new std::unordered_map<std::string, std::string>();
}

Environment::Environment(const Environment& env)
{
	//_vars = new std::unordered_map<std::string, std::string>(env._vars);
	_vars = env._vars;
}

Environment::~Environment()
{
	//delete _vars;
	//_vars = nullptr;
}

Environment& Environment::operator=(const Environment& env)
{
	/*if (_vars == nullptr)
		_vars = new std::unordered_map<std::string, std::string>(env._vars);
	else*/
		_vars = env._vars;
	return *this;
}

void Environment::set(const std::string& name, const std::string& value)
{
	_vars[name] = value;
}

void Environment::unset(const std::string& name)
{
	_vars.erase(name);
}

bool Environment::isset(const std::string& name)
{
	return _vars.count(name) > 0;
}

const std::string& Environment::get(const std::string& name)
{
	if (_vars.count(name))
		return _vars[name];
	else
		// Ugly hack since we haven't got exceptions yet
		return _emptyString;
}
