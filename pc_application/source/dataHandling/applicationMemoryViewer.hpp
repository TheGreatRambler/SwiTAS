#pragma once

#define __STDC_FORMAT_MACROS
#define wxHAS_HUGE_FILES
#include <cstdint>
#include <inttypes.h>
#include <mio.hpp>
#include <system_error>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/string.h>

class ApplicationMemoryManager {
private:
	std::error_code errorCode;
	mio::mmap_sink memoryFile;

	wxDir projectDir;

	wxFileName memMappedFile;

public:
	ApplicationMemoryManager(wxDir dir);

	// Includes startByte and up to endByte but not including it
	// This creates a file in the project folder that's memory mapped
	void setMemoryRegion(uint64_t startByte, uint64_t endByte);

	// Data is automatically sent at every pause, but this gets it manually
	void getData();

	// Signals the switch to stop sending memory and makes getData invalid
	// This also deletes the memory mapped file
	void stopMemoryCollection();
}