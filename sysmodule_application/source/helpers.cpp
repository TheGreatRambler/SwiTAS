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