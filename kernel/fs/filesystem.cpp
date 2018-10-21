//
// Created by Olivier on 01/10/16.
//

#include "fs/filesystem.h"
#include "string.h"

namespace Files
{
	std::string normalize(const char* path)
	{
		std::vector<std::string> parts = split(path);

		bool fromRoot = false;
		if (strlen(path) > 0 && path[0] == '/')
			fromRoot = true;

		size_t i = 0;
		while (i < parts.size())
		{
			if (parts[i] == ".")
				parts.erase(i);
			else if (parts[i] == "..")
			{
				if (i > 0)
				{
					parts.erase(i);
					parts.erase(i-1);
					i--;
				}
				else if (fromRoot)
					parts.erase(i);
				else
					i++;
			}
			else if (parts[i] == "")
				parts.erase(i);
			else
				i++;
		}

		std::string normalized;

		if (strlen(path) > 0 && path[0] == '/')
			normalized += '/';
		
		for (i = 0; i < parts.size(); i++)
		{
			normalized += parts[i];
			if (i+1 < parts.size())
				normalized += '/';
		}

		return normalized;
	}

	std::string normalize(const std::string& path)
	{
		return normalize(path.c_str());
	}

	std::vector<std::string> split(const char* path)
	{
		std::vector<std::string> parts;

		size_t pathLen = strlen(path);
		size_t partStart = 0;
		size_t i = 0;

		if (pathLen > 0 && path[0] == '/')
		{
			i++;
			partStart++;
		}

		while (i < pathLen)
		{
			char c = path[i];
			if (c == '/')
			{
				std::string str(path+partStart, i-partStart);
				parts.push_back(str);
				partStart = i+1;
			}
			i++;
		}

		if (i > partStart)
		{
			std::string str(path+partStart, i-partStart);
			parts.push_back(str);
		}

		return parts;
	}

	/*std::string getPath(const char* path)
	{
		if (path[0] == '/')
			return path;
		else
		{
			std::string basePath = env.get("pwd");
			if (basePath[basePath.length()-1] == '/')
				return normalize(basePath + path);
			else
				return normalize(basePath + '/' + path);
		}
	}

	std::string getPath(const std::string& path)
	{
		return getPath(path.c_str());
	}*/
}