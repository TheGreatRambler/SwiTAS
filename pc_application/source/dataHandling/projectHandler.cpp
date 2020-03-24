#include "projectHandler.hpp"

ProjectHandler::ProjectHandler(DataProcessing* dataProcessingInstance, rapidjson::Document* settings)
	: wxDialog(NULL, wxID_ANY, "Select Project", wxDefaultPosition, wxDefaultSize) {
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
	wxFileName inputsFileName = wxFileName::DirName(projectDir.GetNameWithSep());
	inputsFileName.SetName("inputs");
	inputsFileName.SetExt("bin");

	if(inputsFileName.FileExists()) {
		// Load up the inputs
		wxString filename = inputsFileName.GetFullPath();
		wxFFileInputStream inputsFileStream(filename, "rb");
		wxZlibInputStream inputsDecompressStream(inputsFileStream, wxZLIB_ZLIB | wxZLIB_NO_HEADER);

		wxMemoryOutputStream dataStream;
		dataStream.Write(inputsDecompressStream);

		wxStreamBuffer* streamBuffer = dataStream.GetOutputStreamBuffer();
		uint8_t* bufferPointer       = (uint8_t*)streamBuffer->GetBufferStart();
		std::size_t bufferSize       = streamBuffer->GetBufferSize();

		std::vector<std::shared_ptr<ControllerData>>* inputsList = dataProcessing->getInputsList();

		// Technically, there's one item in there. Remove it
		inputsList->clear();

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
			inputsList->push_back(controllerData);

			sizeRead += sizeOfControllerData;
		}

		// Set the new size and refresh
		dataProcessing->SetItemCount(inputsList->size());
		dataProcessing->Refresh();
	}
}

void ProjectHandler::saveProject() {
	// Save each set of data one by one
	// Serialize the entire vector, first of all, with ZPP
	std::vector<std::shared_ptr<ControllerData>>* inputsList = dataProcessing->getInputsList();

	wxFileName inputsFileName = wxFileName::DirName(projectDir.GetNameWithSep());
	inputsFileName.SetName("inputs");
	inputsFileName.SetExt("bin");

	// Delete file if already present and use binary mode
	wxString filename = inputsFileName.GetFullPath();
	wxFFileOutputStream inputsFileStream(filename, "wb");
	wxZlibOutputStream inputsCompressStream(inputsFileStream, compressionLevel, wxZLIB_ZLIB | wxZLIB_NO_HEADER);

	// Kinda annoying, but actually break up the vector and add each part with the size
	for(auto const& controllerData : *inputsList) {
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