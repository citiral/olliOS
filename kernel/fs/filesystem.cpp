//
// Created by Olivier on 01/10/16.
//

#include "fs/filesystem.h"

std::string Files::getPath(Environment& env, const std::string& path)
{
	if (path[0] == '/')
		return path;
	else
	{
		std::string basePath = env.get("pwd");
		if (basePath[basePath.length()-1] == '/')
			return basePath + path;
		else
			return basePath + '/' + path;
	}
}