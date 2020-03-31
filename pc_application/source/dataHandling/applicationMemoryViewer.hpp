#include "applicationMemoryViewer.hpp"

ApplicationMemoryManager::ApplicationMemoryManager() { }

void ApplicationMemoryManager::setMemoryRegion(uint64_t startByte, uint64_t endByte) {
	memMappedFile.SetPath("Idunno");
	// Create dir if needed
	// memMappedFile.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	memMappedFile.SetName(wxString::Format("byte_%" PRIu64 "_to_byte_%" PRIu64));
	memMappedFile.SetExt("bin");

	wxFile theFile;
	// Allow reading and writing by all users
	theFile.Create(memMappedFile.GetFullPath(), true, wxS_DEFAULT);
	// Triggers sparse file creation to get the file created at the right size
	// https://stackoverflow.com/questions/7896035/c-make-a-file-of-a-specific-size
	theFile.Seek((endByte - startByte) - 1);
	theFile.Write("", 1);
	theFile.Close();

	// Map this file as memory
	// https://github.com/mandreyel/mio
	memoryFile = mio::make_mmap_sink(memMappedFile.GetFullPath(), 0, mio::map_entire_file, errorCode);
}

void ApplicationMemoryManager::getData() {
	// Set data like a vector
	// memoryFile[i] = "data";
	// This is set upon reading the network
	// This will write the data to the file
	// memoryFile.sync(errorCode);
	// Upon being written, the hex editor will read the data on update
}

void ApplicationMemoryManager::stopMemoryCollection() {
	memoryFile.unmap();
	// Close the file
	// I don't know how to delete yet
}