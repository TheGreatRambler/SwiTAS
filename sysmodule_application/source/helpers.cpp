#include "helpers.hpp"

void HELPERS::getMemUsage(uint64_t* memAvaliable, uint64_t* memUsed) {
#ifdef __SWITCH__
	svcGetInfo(memAvaliable, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
	svcGetInfo(memUsed, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
#endif
}

void HELPERS::readFullFileData(FILE* file, void* bufPtr, int size) {
	int sizeActuallyRead = 0;
	uint8_t* buf         = (uint8_t*)bufPtr;

	while(sizeActuallyRead != size) {
		int bytesRead = fread(&buf[sizeActuallyRead], size - sizeActuallyRead, 1, file);
		sizeActuallyRead += bytesRead;
	}
}

void HELPERS::replaceInString(std::string input, std::string initial, std::string final) {
	int index;
	while((index = input.find(initial)) != std::string::npos) {
		input.replace(index, final.length(), final);
	}
}

// https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
std::string HELPERS::ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

std::string HELPERS::getExecutableDir() {
	char path[1024];
	std::string ret;
#ifdef _WIN32
	DWORD length = GetModuleFileNameA(NULL, path, sizeof(path));
	PathRemoveFileSpecA(path);
	ret = std::string(path, strlen(path));
#endif
#ifdef __linux__
	ssize_t length = readlink("/proc/self/exe", path, sizeof(path));
	path[length]   = '\0';
	char* dir      = dirname(path);
	ret            = std::string(dir, strlen(dir));
#endif
#ifdef __APPLE__
	_NSGetExecutablePath(path, sizeof(path));
	char resolvedPath[1024];
	realpath(path, resolvedPath);
	char* dir = dirname(resolvedPath);
	ret       = std::string(dir, strlen(dir));
#endif
	return ret;
}

#ifdef YUZU
std::string HELPERS::exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if(!pipe) {
		return "";
	}
	while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}
#endif