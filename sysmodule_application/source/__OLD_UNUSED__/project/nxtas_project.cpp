#include <cstdio>

#include "nxtas_project.hpp"
#include "serialize_unserialize_data.hpp"

NxTASProject::NxTASProject() {}

std::shared_ptr<InputData> NxTASProject::getFrameData(uint32_t frame) {
	// This reference is used both to read and write data
	// This function makes a very important assumption that all
	// Writing of frame data will be done before the next call to getFrameData
	// Determine if input is already loaded into memory
	uint16_t pageNum = floor((double)frame / 60);
	if(inputPages.count(pageNum) == 0) {
		// The frame is not loaded into memory and it needs to be created
		// Load into memory now
		loadFramesFromFile(pageNum);
	}

	std::shared_ptr<InputData> inputData = inputPages[pageNum].inputs[frame % 60];

	// Determine what pages need to be unloaded
	for(auto& page : inputPages) {
		if(page.first != pageNum) {
			// This page has not been read, check if it needs to be deleted
			if(page.second.accessesSinceLastRead == timesBeforeDeleted) {
				// Delete it now
				inputPages.erase(inputPages.find(page.first));
			} else {
				// Just increment the accesses
				page.second.accessesSinceLastRead++;
			}
		}
	}

	// Finally return the inputData
	return inputData;
}

void NxTASProject::loadFramesFromFile(uint16_t pageNum) {
	// We only need to know if the file exists
	bool fileExists = access(Helpers::getPageBinName(projectFolder, pageNum).c_str(), F_OK) == -1;
	// Create inputPage
	InputsPage inputsPage;
	inputsPage.accessesSinceLastRead = 0;
	if(fileExists) {
		// The file exists so the data needs to be read
		uint8_t fileBeingRead = true;
		FILE* fptr;
		std::string binName = Helpers::getPageBinName(projectFolder, pageNum);
		// Don't overwrite
		fptr = fopen(binName.c_str(), "rb");
		while(fileBeingRead) {
			// Get the size of the following data
			// The index read is actually the index compared to the start of the page
			uint16_t dataSize;
			// Read from the file
			// It automatically advances the read index
			fread(&dataSize, sizeof(dataSize), 1, fptr);
			// Convert from network endianness
			dataSize = ntohs(dataSize);
			// Read the index of the frame next
			uint8_t frameNum;
			fread(&frameNum, sizeof(frameNum), 1, fptr);
			// Now, get the index data itself
			uint8_t* inputsData;
			fread(inputsData, dataSize, 1, fptr);
			// Get the data itself now
			std::shared_ptr<InputData> frameInput = SerializeProtocol::binaryToFrame<InputData>(inputsData, dataSize);
			// Add to the page nows
			inputsPage.inputs[frameNum] = frameInput;
			// Feof will return true when the end of the file is reached and
			// Set filebeingread to false
			fileBeingRead = !feof(fptr);
		}
		fclose(fptr);
	}
	// If the file doesn't exist, there will be no inputs
	inputPages[pageNum] = inputsPage;
}

void NxTASProject::writeFramesToFile(uint16_t pageNum) {
	// Check whether there are any frames to save anyway
	if(inputPages[pageNum].inputs.size() != 0) {
		// The frames need to be written to file because they don't need to be used right now
		FILE* fptr;
		std::string binName = Helpers::getPageBinName(projectFolder, pageNum);
		// Not sure whether this creates the folder too
		// Specifically overwrites the content with the 'w' flag
		fptr = fopen(binName.c_str(), "wb+");
		/*if(fptr == NULL) {
			// Need to create the folder as well
			Helpers::mkpath((projectFolder + "/frames").c_str(), 0755);
			fptr = fopen(binName.c_str(), "wb");
		}*/
		for(auto const& input : inputPages[pageNum].inputs) {
			uint16_t size;
			uint8_t* data;
			// Serialize the frame here
			SerializeProtocol::frameToBinary(input.second, data, &size);
			// Add to file
			// Write size of the frame first
			// Make the frame the right endianness
			uint16_t rightSize = htons(size);
			fwrite(&rightSize, sizeof(rightSize), 1, fptr);
			// Write the frame number next
			fwrite(&input.first, sizeof(input.first), 1, fptr);
			// Finally, write the frame itself
			fwrite(data, size, 1, fptr);
		}
		// Close the file because writing is done
		fclose(fptr);
	}
}

void NxTASProject::setProjectName(std::string name) {
	projectName   = name;
	projectFolder = "/nxtas/projects/" + projectName;
	savestateHandler.setProjectFolder(projectFolder);
}

NxTASProject::~NxTASProject() {}