#include "applicationMemoryViewer.hpp"

ApplicationMemoryManager::ApplicationMemoryManager(wxString dir, std::shared_ptr<CommunicateWithNetwork> networkImp) {
	projectDir      = dir;
	networkInstance = networkImp;
}

void ApplicationMemoryManager::addMemoryRegion(uint64_t startByte, uint64_t size) {
	std::shared_ptr<MemorySection> memorySection = std::make_shared<MemorySection>();

	memorySection->startByte = startByte;
	memorySection->size      = size;

	memorySection->memMappedFile.SetPath(projectDir + "applicationMemory");
	// Create dir if needed
	memorySection->memMappedFile.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
#ifdef __WXMSW__
	memorySection->memMappedFile.SetName(wxString::Format("byte_%I64u_to_byte_%I64u", startByte, startByte + size));
#else
	memorySection->memMappedFile.SetName(wxString::Format("byte_%llu_to_byte_%llu", startByte, startByte + size));
#endif
	memorySection->memMappedFile.SetExt("bin");

	wxFile theFile;
	// Allow reading and writing by all users
	theFile.Create(memorySection->memMappedFile.GetFullPath(), true, wxS_DEFAULT);
	// Triggers sparse file creation to get the file created at the right size
	// https://stackoverflow.com/questions/7896035/c-make-a-file-of-a-specific-size
	theFile.Seek(size - 1);
	theFile.Write("", 1);
	theFile.Close();

	// Map this file as memory
	// https://github.com/mandreyel/mio
	memorySection->memoryFile = mio::make_mmap_sink(memorySection->memMappedFile.GetFullPath().ToStdString(), 0, mio::map_entire_file, errorCode);

	memorySections.insert(std::pair<std::string, std::shared_ptr<MemorySection>>(getID(startByte, size), memorySection));

	ADD_TO_QUEUE(SendTrackMemoryRegion, networkInstance, {
		data.startByte = startByte;
		data.size      = size;
	})
}

void ApplicationMemoryManager::getData(uint64_t startByte, uint64_t size) {
	// Set data like a vector
	// memoryFile[i] = "data";
	// This is set upon reading the network
	// This will write the data to the file
	// memoryFile.sync(errorCode);
	// Upon being written, the hex editor will read the data on update
}

void ApplicationMemoryManager::stopMemoryCollection(uint64_t startByte, uint64_t size) {
	memorySections[getID(startByte, size)]->memoryFile.unmap();
	// Close the file
	// I don't know how to delete yet
}