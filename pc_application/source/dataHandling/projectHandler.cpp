#include "projectHandler.hpp"

ProjectHandler::ProjectHandler(DataProcessing* dataProcessingInstance, rapidjson::Document* settings)
	: wxDialog(NULL, wxID_ANY, "Select Project", wxDefaultPosition, wxDefaultSize) {
	// OKAY
	// Big brain time
	// I know how savestate hooks will work
	// Savestate hooks will represent the START of a block of inputs
	// So, the inputs viewable in the wxListCtrl will be just one block
	// Savestate hooks can be created at any time
	// They don't have to be created with the current frame, they are independent of the current frame data
	// Each block of inputs is in its own file, for git purposes
	// Get global settings file
	dataProcessing = dataProcessingInstance;
	mainSettings   = settings;

	mainSizer = new wxBoxSizer(wxVERTICAL);

	projectList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);

	rapidjson::GenericArray<false, rapidjson::Value> recentProjectsArray = (*mainSettings)["recentProjects"].GetArray();

	std::size_t listboxSize = recentProjectsArray.Size() + 1;
	wxString listboxItems[listboxSize];
	std::size_t listboxIndex = 0;

	for(auto const& recentProject : recentProjectsArray) {
		wxString recentProjectItem = wxString::Format("%s - %s", wxString::FromUTF8(recentProject["projectName"].GetString()), wxString::FromUTF8(recentProject["projectDirectory"].GetString()));
		listboxItems[listboxIndex] = recentProjectItem;
		listboxIndex++;
	}

	listboxItems[listboxIndex] = createNewProjectText;

	projectList->InsertItems(listboxSize, listboxItems, 0);

	projectList->Bind(wxEVT_LISTBOX, &ProjectHandler::onClickProject, this);

	mainSizer->Add(projectList, 0, wxEXPAND | wxALL, 5);

	// Print create project input

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void ProjectHandler::onClickProject(wxCommandEvent& event) {
	rapidjson::GenericArray<false, rapidjson::Value> recentProjectsArray = (*mainSettings)["recentProjects"].GetArray();
	int selectedProject                                                  = event.GetInt();
	if(selectedProject == recentProjectsArray.Size()) {
		// Open up a new project dialog
		wxDirDialog dlg(NULL, "Choose Project Directory", "", wxDD_DEFAULT_STYLE);
		if(dlg.ShowModal() == wxID_OK) {
			// Directory chosen
			projectDir.Open(dlg.GetPath());
			recentProjectChoice = -1;
			projectName         = "Unnamed";
			projectChosen       = true;
			Close(true);
		}
	} else {
		// It's a project folder
		projectDir.Open(recentProjectsArray[selectedProject]["projectDirectory"].GetString());
		recentProjectChoice = selectedProject;
		projectName         = std::string(recentProjectsArray[selectedProject]["projectName"].GetString());
		// Fill up the data found here
		loadProject();
		projectChosen = true;
		Close(true);
	}
}

void ProjectHandler::loadProject() {
	wxFileName settingsFileName = getProjectStart();
	settingsFileName.SetName("settings");
	settingsFileName.SetExt("json");

	// Load up JSON file with info
	// First, find each savestate hook block
	rapidjson::Document jsonSettings = HELPERS::getSettingsFile(settingsFileName.GetFullPath().ToStdString());

	rapidjson::GenericArray<false, rapidjson::Value> savestateBlocksArray = jsonSettings["savestateBlocks"].GetArray();
	std::vector<std::string> savestateFileNames(savestateBlocksArray.Size());

	for(auto const& savestateBlock : savestateBlocksArray) {
		int index                 = savestateBlock["index"].GetInt();
		savestateFileNames[index] = std::string(savestateBlock["filename"].GetString());
	}

	// The savestate hooks to create
	AllSavestateHookBlocks savestateHookBlocks(savestateFileNames.size());

	std::size_t index = 0;
	for(auto const& savestateFileName : savestateFileNames) {
		wxString path = projectDir.GetNameWithSep() + wxFileName::GetPathSeparator() + wxString(savestateFileName);
		if(wxFileName(path).FileExists()) {

			// Load up the inputs
			wxFFileInputStream inputsFileStream(path, "rb");
			wxZlibInputStream inputsDecompressStream(inputsFileStream, wxZLIB_ZLIB | wxZLIB_NO_HEADER);

			wxMemoryOutputStream dataStream;
			dataStream.Write(inputsDecompressStream);

			wxStreamBuffer* streamBuffer = dataStream.GetOutputStreamBuffer();
			uint8_t* bufferPointer       = (uint8_t*)streamBuffer->GetBufferStart();
			std::size_t bufferSize       = streamBuffer->GetBufferSize();

			// THIS REQUIRES ENTIRELY DIFFERENT FILE HANDLING TODO
			// Now have to handle by savestate hook block

			SavestateHookBlock block = std::make_shared<std::vector<std::shared_ptr<ControllerData>>>();

			// Loop through each part and unserialize it
			// This is 0% endian safe
			std::size_t sizeRead = 0;
			while(sizeRead != bufferSize) {
				// Find the size part first
				uint8_t sizeOfControllerData = bufferPointer[sizeRead];
				sizeRead += sizeof(sizeOfControllerData);
				// Load the data
				std::shared_ptr<ControllerData> controllerData = std::make_shared<ControllerData>();

				serializeProtocol.binaryToData<ControllerData>(*controllerData, &bufferPointer[sizeRead], sizeOfControllerData);
				// For now, just add each frame one at a time, no optimization
				block->push_back(controllerData);

				sizeRead += sizeOfControllerData;
			}

			savestateHookBlocks[index] = block;
			index++;
		}
	}

	// Set dataProcessing
	dataProcessing->setAllSavestateHookBlocks(savestateHookBlocks);
}

void ProjectHandler::saveProject() {
	// Save each set of data one by one

	AllSavestateHookBlocks& savestateHookBlocks = dataProcessing->getAllSavestateHookBlocks();

	rapidjson::Document settingsJSON;
	settingsJSON.SetObject();

	rapidjson::Value savestateHooksJSON(rapidjson::kArrayType);

	std::size_t index = 0;
	for(auto const& savestateHookBlock : savestateHookBlocks) {

		wxFileName inputsFileName = getProjectStart();
		inputsFileName.SetName(wxString::Format("savestate_block_%lld", index));
		inputsFileName.SetExt("bin");

		// Delete file if already present and use binary mode
		wxString filename = inputsFileName.GetFullPath();
		wxFFileOutputStream inputsFileStream(filename, "wb");
		wxZlibOutputStream inputsCompressStream(inputsFileStream, compressionLevel, wxZLIB_ZLIB | wxZLIB_NO_HEADER);

		// Kinda annoying, but actually break up the vector and add each part with the size
		for(auto const& controllerData : *savestateHookBlock) {
			uint8_t* data;
			std::size_t dataSize;
			serializeProtocol.dataToBinary<ControllerData>(*controllerData, &data, &dataSize);
			uint8_t sizeToPrint = (uint8_t)dataSize;
			// Probably endian issues
			inputsCompressStream.WriteAll(&sizeToPrint, sizeof(sizeToPrint));
			inputsCompressStream.WriteAll(data, dataSize);
		}

		inputsCompressStream.Sync();
		inputsCompressStream.Close();
		inputsFileStream.Close();

		inputsFileName.MakeRelativeTo(projectDir.GetNameWithSep());

		// Add the item in the savestateHooks JSON
		rapidjson::Value savestateHookJSON(rapidjson::kObjectType);

		rapidjson::Value savestateHookPath;
		wxString path = inputsFileName.GetFullPath();
		savestateHookPath.SetString(path.c_str(), strlen(path.c_str()), settingsJSON.GetAllocator());

		rapidjson::Value savestateHookIndex;
		savestateHookIndex.SetUint(index);

		savestateHookJSON.AddMember("filename", savestateHookPath, settingsJSON.GetAllocator());
		savestateHookJSON.AddMember("index", savestateHookIndex, settingsJSON.GetAllocator());

		savestateHooksJSON.PushBack(savestateHookJSON, settingsJSON.GetAllocator());

		index++;
	}

	settingsJSON.AddMember("savestateBlocks", savestateHooksJSON, settingsJSON.GetAllocator());

	// Write out the savestate blocks in the settings, TODO, add more
	wxFileName settingsFileName = getProjectStart();
	settingsFileName.SetName("settings");
	settingsFileName.SetExt("json");

	wxFFileOutputStream normalSettings(settingsFileName.GetFullPath(), "w");
	rapidjson::StringBuffer settingsSb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> settingsWriter(settingsSb);
	settingsWriter.SetIndent('\t', 1);
	settingsJSON.Accept(settingsWriter);

	normalSettings.WriteAll(settingsSb.GetString(), settingsSb.GetLength());
	normalSettings.Close();

	rapidjson::GenericArray<false, rapidjson::Value> recentProjectsArray = (*mainSettings)["recentProjects"].GetArray();
	// Add to recent projects list if not yet there, otherwise modify
	if(recentProjectChoice == -1) {
		// Add new
		rapidjson::Value newRecentProject(rapidjson::kObjectType);

		rapidjson::Value name;
		name.SetString(projectName.c_str(), strlen(projectName.c_str()), mainSettings->GetAllocator());

		rapidjson::Value directory;
		wxString dirString = projectDir.GetNameWithSep();
		directory.SetString(dirString.mb_str(), dirString.length(), mainSettings->GetAllocator());

		newRecentProject.AddMember("projectDirectory", directory, mainSettings->GetAllocator());
		newRecentProject.AddMember("projectName", name, mainSettings->GetAllocator());

		// I think it's a reference, not sure
		recentProjectsArray.PushBack(newRecentProject, mainSettings->GetAllocator());
	} else {
		// Modify existing values
		wxString dirString = projectDir.GetNameWithSep();
		recentProjectsArray[recentProjectChoice]["projectDirectory"].SetString(dirString.c_str(), dirString.length(), mainSettings->GetAllocator());

		recentProjectsArray[recentProjectChoice]["projectName"].SetString(projectName.c_str(), projectName.size(), mainSettings->GetAllocator());
	}

	// Additionally, save the mainSettings and overwrite
	wxFFileOutputStream settingsFileStream("../mainSettings.json", "w");

	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
	writer.SetIndent('\t', 1);
	mainSettings->Accept(writer);

	settingsFileStream.WriteAll(sb.GetString(), sb.GetLength());
	settingsFileStream.Close();
}

void ProjectHandler::createTempProjectDir() {
	// Open standard documents folder
	wxString documentsFolder = wxStandardPaths::Get().GetDocumentsDir();
	wxFileName dir(documentsFolder);
	dir.AppendDir(wxString::Format("TAS_PROJECT_%lld", wxGetLocalTimeMillis().GetValue()));
	projectDir.Open(dir.GetFullPath());
	dir.Mkdir();
}

ProjectHandler::~ProjectHandler() {}