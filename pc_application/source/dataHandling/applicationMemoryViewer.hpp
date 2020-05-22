#pragma once

#define wxHAS_HUGE_FILES
#include <cstdint>
#include <cstring>
#include <mio.hpp>
#include <system_error>
#include <unordered_map>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/longlong.h>
#include <wx/string.h>

#include "../sharedNetworkCode/networkInterface.hpp"

struct MemorySection {
	uint64_t startByte;
	uint64_t size;
	mio::mmap_sink memoryFile;
	wxFileName memMappedFile;
};

class ApplicationMemoryManager {
private:
	std::error_code errorCode;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	wxString projectDir;

	std::unordered_map<std::string, std::shared_ptr<MemorySection>> memorySections;

	std::string getID(uint64_t startByte, uint64_t size) {
		char buffer[50];
		int n = sprintf(buffer, "%llu-%llu", startByte, size);
		return std::string(buffer, n);
	}

public:
	ApplicationMemoryManager(wxString dir, std::shared_ptr<CommunicateWithNetwork> networkImp);

	// Includes startByte and up to endByte but not including it
	// This creates a file in the project folder that's memory mapped
	void addMemoryRegion(uint64_t startByte, uint64_t size);

	// Data is automatically sent at every pause, but this gets it manually
	void getData(uint64_t startByte, uint64_t size);

	// Signals the switch to stop sending memory and makes getData invalid
	// This also deletes the memory mapped file
	void stopMemoryCollection(uint64_t startByte, uint64_t size);
};