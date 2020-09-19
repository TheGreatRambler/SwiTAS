#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace HELPERS {
	void getMemUsage(uint64_t* memAvaliable, uint64_t* memUsed);

	void readFullFileData(FILE* file, void* bufPtr, int size);

	void replaceInString(std::string input, std::string initial, std::string final);

	// https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

	std::string getExecutableDir();
}