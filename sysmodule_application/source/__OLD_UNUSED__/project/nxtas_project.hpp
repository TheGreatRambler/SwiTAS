#pragma once

#define __YAS_LITTLE_ENDIAN

#include <arpa/inet.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>

#include "button_data.hpp"
#include "helpers.hpp"

#include "../communication/communicate_with_pc.hpp"
#include "handle_savestates.hpp"

class NxTASProject {
private:
	// The first key is the starting page, 0 for 0-59 frames, 1 for 60-119, etc...
	std::unordered_map<uint16_t, InputsPage> inputPages;

	// The project name, determines the folder name of the project
	std::string projectName;
	std::string projectFolder;

	// The number of times a page is not read before it is deleted automatically
	static constexpr uint8_t timesBeforeDeleted = 3;

	// The classes that handle more specific data
	CommunicateWithPC communicateWithPc;
	SavestateHandler savestateHandler;

public:
	NxTASProject();

	// This function will load a page from memory if need be and will
	// otherwise use the filesystem
	std::shared_ptr<InputData> getFrameData(uint32_t frame);

	// Uses YAS to load the framedata
	void loadFramesFromFile(uint16_t pageNum);

	// This writes the page to a file and closes the page
	void writeFramesToFile(uint16_t pageNum);

	void setProjectName(std::string name);

	~NxTASProject();
};