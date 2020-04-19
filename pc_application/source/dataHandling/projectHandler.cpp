#include "projectHandler.hpp"

ProjectHandler::ProjectHandler(wxFrame* parent, DataProcessing* dataProcessingInstance, rapidjson::Document* settings) {
	dataProcessing = dataProcessingInstance;
	mainSettings   = settings;
	parentFrame    = parent;
	// TODO this is simiar to the joysticks submenu
	videoComparisonEntriesMenu = new wxMenu();

	dataProcessing->setSelectedFrameCallbackVideoViewer(std::bind(&ProjectHandler::updateVideoComparisonViewers, this, std::placeholders::_1));
}

void ProjectHandler::loadProject() {
	// TODO, this does no error checking

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

	for(auto const& videoEntryJson : jsonSettings["videos"].GetArray()) {
		std::shared_ptr<VideoEntry> videoEntry = std::make_shared<VideoEntry>();

		videoEntry->videoUrl         = std::string(videoEntryJson["videoUrl"].GetString());
		videoEntry->videoName        = std::string(videoEntryJson["videoName"].GetString());
		videoEntry->videoFilename    = std::string(videoEntryJson["videoFilename"].GetString());
		videoEntry->videoMetadata    = std::string(videoEntryJson["videoMetadata"].GetString());
		videoEntry->videoPath        = HELPERS::makeFromRelative(std::string(videoEntryJson["videoPath"].GetString()), projectDir.GetName().ToStdString());
		videoEntry->videoIndexerPath = HELPERS::makeFromRelative(std::string(videoEntryJson["videoIndexerPath"].GetString()), projectDir.GetName().ToStdString());

		videoComparisonEntries.push_back(videoEntry);
	}

	// Set dataProcessing
	dataProcessing->setAllSavestateHookBlocks(savestateHookBlocks);
}

void ProjectHandler::saveProject() {
	if(projectWasLoaded) {
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

		rapidjson::Value recentVideoEntries(rapidjson::kArrayType);
		for(auto const& videoEntry : videoComparisonEntries) {
			rapidjson::Value newRecentVideo(rapidjson::kObjectType);

			std::string projectDirectory = projectDir.GetName().ToStdString();
			videoEntry->videoPath        = HELPERS::makeRelative(videoEntry->videoPath, projectDirectory);
			videoEntry->videoIndexerPath = HELPERS::makeRelative(videoEntry->videoIndexerPath, projectDirectory);

			std::replace(videoEntry->videoPath.begin(), videoEntry->videoPath.end(), '\\', '/');
			std::replace(videoEntry->videoIndexerPath.begin(), videoEntry->videoIndexerPath.end(), '\\', '/');

			rapidjson::Value url;
			url.SetString(videoEntry->videoUrl.c_str(), videoEntry->videoUrl.size(), settingsJSON.GetAllocator());

			rapidjson::Value name;
			name.SetString(videoEntry->videoName.c_str(), videoEntry->videoName.size(), settingsJSON.GetAllocator());

			rapidjson::Value filename;
			filename.SetString(videoEntry->videoFilename.c_str(), videoEntry->videoFilename.size(), settingsJSON.GetAllocator());

			rapidjson::Value metadataString;
			metadataString.SetString(videoEntry->videoMetadata.c_str(), videoEntry->videoMetadata.size(), settingsJSON.GetAllocator());

			rapidjson::Value videoPath;
			videoPath.SetString(videoEntry->videoPath.c_str(), videoEntry->videoPath.size(), settingsJSON.GetAllocator());

			rapidjson::Value videoIndexerPath;
			videoIndexerPath.SetString(videoEntry->videoIndexerPath.c_str(), videoEntry->videoIndexerPath.size(), settingsJSON.GetAllocator());

			newRecentVideo.AddMember("videoUrl", url, settingsJSON.GetAllocator());
			newRecentVideo.AddMember("videoName", name, settingsJSON.GetAllocator());
			newRecentVideo.AddMember("videoFilename", filename, settingsJSON.GetAllocator());
			newRecentVideo.AddMember("videoMetadata", metadataString, settingsJSON.GetAllocator());
			newRecentVideo.AddMember("videoPath", videoPath, settingsJSON.GetAllocator());
			newRecentVideo.AddMember("videoIndexerPath", videoIndexerPath, settingsJSON.GetAllocator());

			recentVideoEntries.PushBack(newRecentVideo, settingsJSON.GetAllocator());
		}

		settingsJSON.AddMember("savestateBlocks", savestateHooksJSON, settingsJSON.GetAllocator());
		settingsJSON.AddMember("videos", recentVideoEntries, settingsJSON.GetAllocator());

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

		// Run some housekeeping to delete recent project entries that don't exist
		/*
		auto it = recentProjectsArray.Begin();
		while(it != recentProjectsArray.End()) {
			wxFileName dir((*it)["projectDirectory"].GetString());
			if(!dir.DirExists()) {
				// Erase because the directory doesn't exist
				it = recentProjectsArray.Erase(it);
			} else {
				++it;
			}
		}
		*/

		// Additionally, save the mainSettings and overwrite
		wxFFileOutputStream settingsFileStream("../mainSettings.json", "w");

		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		writer.SetIndent('\t', 1);
		mainSettings->Accept(writer);

		settingsFileStream.WriteAll(sb.GetString(), sb.GetLength());
		settingsFileStream.Close();
	}
}

void ProjectHandler::openUpVideoComparisonViewer(int index) {
	wxString projDir              = projectDir.GetName();
	VideoComparisonViewer* viewer = new VideoComparisonViewer(parentFrame, std::bind(&ProjectHandler::closeVideoComparisonViewer, this, std::placeholders::_1), mainSettings, videoComparisonEntries, projDir);
	videoComparisonViewers.push_back(viewer);
	if(index != videoComparisonEntries.size()) {
		// Old video, load with the preset
		viewer->openWithRecent(index);
	}
	viewer->Show(true);
}

void ProjectHandler::closeVideoComparisonViewer(VideoComparisonViewer* viewer) {
	// Seek for the instance in the vector and remove it
	// Don't delete it, as the window did that itself
	for(std::size_t i = 0; i < videoComparisonViewers.size(); i++) {
		// If this is the window, remove it from the list
		if(videoComparisonViewers[i] == viewer) {
			videoComparisonViewers.erase(videoComparisonViewers.begin() + i);
			break;
		}
	}
}

void ProjectHandler::updateVideoComparisonViewers(FrameNum delta) {
	for(std::size_t i = 0; i < videoComparisonViewers.size(); i++) {
		videoComparisonViewers[i]->seekRelative(delta);
	}
}

void ProjectHandler::onRecentVideosMenuOpen(wxMenuEvent& event) {
	// Remove all current items
	int currentNumOfItems = videoComparisonEntriesMenu->GetMenuItemCount();
	for(int i = 0; i < currentNumOfItems; i++) {
		videoComparisonEntriesMenu->Delete(i + videoComparisonEntriesMenuIDBase);
	}

	for(int i = 0; i < videoComparisonEntries.size(); i++) {
		wxString printString = wxString::FromUTF8(videoComparisonEntries[i]->videoName + " " + videoComparisonEntries[i]->videoMetadata + " " + videoComparisonEntries[i]->videoUrl);
		videoComparisonEntriesMenu->Append(i + videoComparisonEntriesMenuIDBase, printString);
	}

	videoComparisonEntriesMenu->Append(videoComparisonEntriesMenu->GetMenuItemCount() + videoComparisonEntriesMenuIDBase, "Open new video");
}

void ProjectHandlerWindow::createTempProjectDir() {
	// Open standard documents folder
	wxString documentsFolder = wxStandardPaths::Get().GetDocumentsDir();
	wxFileName dir(documentsFolder);
	dir.AppendDir(wxString::Format("TAS_PROJECT_%lld", wxGetLocalTimeMillis().GetValue()));
	projectHandler->setProjectDir(dir.GetFullPath());
}

ProjectHandlerWindow::ProjectHandlerWindow(std::shared_ptr<ProjectHandler> projHandler, rapidjson::Document* settings)
	: wxDialog(NULL, wxID_ANY, "Select Project", wxDefaultPosition, wxDefaultSize) {
	projectHandler = projHandler;
	mainSettings   = settings;

	mainSizer = new wxBoxSizer(wxVERTICAL);

	projectList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);

	rapidjson::GenericArray<false, rapidjson::Value> recentProjectsArray = projectHandler->getRecentProjects();

	std::size_t listboxSize = recentProjectsArray.Size() + 2;
	wxString listboxItems[listboxSize];
	std::size_t listboxIndex = 0;

	for(auto const& recentProject : recentProjectsArray) {
		wxString projectName       = wxString::FromUTF8(recentProject["projectName"].GetString());
		wxString projectDirectory  = wxString::FromUTF8(recentProject["projectDirectory"].GetString());
		wxString recentProjectItem = wxString::Format("%s - %s", projectName, projectDirectory);
		listboxItems[listboxIndex] = recentProjectItem;
		listboxIndex++;
	}

	listboxItems[listboxIndex]     = loadExistingProjectText;
	listboxItems[listboxIndex + 1] = createNewProjectText;

	projectList->InsertItems(listboxSize, listboxItems, 0);

	// Add splash screen
	std::string splashScreenPath = HELPERS::resolvePath((*mainSettings)["ui"]["splashScreen"].GetString());
	wxImage splashImage(wxString::FromUTF8(splashScreenPath));
	DrawingCanvasBitmap* splashScreen = new DrawingCanvasBitmap(this, splashImage.GetSize());
	splashScreen->setBitmap(new wxBitmap(splashImage));

	mainSizer->Add(splashScreen, 0, wxSHAPED | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(projectList, 0, wxEXPAND | wxALL);

	projectList->Bind(wxEVT_LISTBOX, &ProjectHandlerWindow::onClickProject, this);

	// Print create project input

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void ProjectHandlerWindow::onClickProject(wxCommandEvent& event) {
#ifdef __WXGTK__
	if(!projectListFirstTime) {
#endif
		rapidjson::GenericArray<false, rapidjson::Value> recentProjectsArray = projectHandler->getRecentProjects();
		int selectedProject                                                  = event.GetInt();
		if(selectedProject == recentProjectsArray.Size()) {
			// Open up a load project dialog
			wxDirDialog dlg(NULL, "Choose Project Directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
			if(dlg.ShowModal() == wxID_OK) {
				projectHandler->setProjectDir(dlg.GetPath());
				projectHandler->setRecentProjectChoice(-1);
				projectHandler->setProjectName("Unnamed");
				projectChosen       = true;
				wasClosedForcefully = false;

				projectHandler->loadProject();
				Close(true);
			} else {
				projectList->Deselect(selectedProject);
			}
		} else if(selectedProject == recentProjectsArray.Size() + 1) {
			// Open up a new project dialog
			wxDirDialog dlg(NULL, "Choose Project Directory", "", wxDD_DEFAULT_STYLE);
			if(dlg.ShowModal() == wxID_OK) {
				projectHandler->setProjectDir(dlg.GetPath());
				projectHandler->setRecentProjectChoice(-1);
				projectHandler->setProjectName("Unnamed");
				projectChosen       = true;
				wasClosedForcefully = false;

				Close(true);
			} else {
				projectList->Deselect(selectedProject);
			}
		} else {
			// It's a project folder
			wxFileName dir(recentProjectsArray[selectedProject]["projectDirectory"].GetString());
			if(dir.DirExists()) {
				projectHandler->setProjectDir(dir.GetFullPath());
				projectHandler->setRecentProjectChoice(selectedProject);
				projectHandler->setProjectName(std::string(recentProjectsArray[selectedProject]["projectName"].GetString()));
				projectChosen       = true;
				wasClosedForcefully = false;

				projectHandler->loadProject();
				Close(true);
			} else {
				// This dir doesn't exist, ask the user about it
				wxMessageDialog deleteDialog(this, "This directory doesn't exist, should it be removed from the recent projects list?", "Delete recent project entry", wxYES_NO | wxCANCEL | wxNO_DEFAULT | wxICON_ERROR);
				int res = deleteDialog.ShowModal();
				projectList->Deselect(selectedProject);
				if(res == wxID_YES) {
					// Delete the entry
					projectList->Delete(selectedProject);
					projectList->Refresh();
					projectHandler->removeRecentProject(selectedProject);
				}
			}
		}
#ifdef __WXGTK__
	} else {
		projectList->Deselect(0);
		projectListFirstTime = false;
	}
#endif
}