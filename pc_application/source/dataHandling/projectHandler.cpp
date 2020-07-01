#include "projectHandler.hpp"

ProjectHandler::ProjectHandler(wxFrame* parent, DataProcessing* dataProcessingInstance, rapidjson::Document* settings) {
	dataProcessing = dataProcessingInstance;
	mainSettings   = settings;
	parentFrame    = parent;
	// TODO this is simiar to the joysticks submenu
	videoComparisonEntriesMenu = new wxMenu();

	// Initialize it as empty
	lastEnteredFtpPath = "";

	// Get recent settings
	recentSettings = HELPERS::getSettingsFile(HELPERS::getMainSettingsPath("switas_recent").GetFullPath().ToStdString());

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

	auto playersArray = jsonSettings["players"].GetArray();

	// The players to create
	AllPlayers players(playersArray.Size());

	uint8_t playerIndex = 0;
	for(auto const& player : playersArray) {
		auto savestateBlocksArray = player["savestateBlocks"].GetArray();

		std::shared_ptr<AllSavestateHookBlocks> savestateHookBlocks = std::make_shared<AllSavestateHookBlocks>(savestateBlocksArray.Size());

		SavestateBlockNum savestateHookIndex = 0;
		for(auto const& savestate : savestateBlocksArray) {
			auto branchesArray = savestate["branches"].GetArray();

			std::shared_ptr<SavestateHook> savestateHook = std::make_shared<SavestateHook>();

			for(auto const& branch : branchesArray) {
				wxString path = projectDir.GetPathWithSep() + wxString::FromUTF8(branch["filename"].GetString());
				if(wxFileName(path).FileExists()) {
					// Load up the inputs
					wxFFileInputStream inputsFileStream(path, "rb");
					wxZlibInputStream inputsDecompressStream(inputsFileStream, wxZLIB_ZLIB);

					wxMemoryOutputStream dataStream;
					dataStream.Write(inputsDecompressStream);

					wxStreamBuffer* streamBuffer = dataStream.GetOutputStreamBuffer();
					uint8_t* bufferPointer       = (uint8_t*)streamBuffer->GetBufferStart();
					std::size_t bufferSize       = streamBuffer->GetBufferSize();

					BranchData inputs = std::make_shared<std::vector<std::shared_ptr<ControllerData>>>();

					// Loop through each part and unserialize it
					// This is 0% endian safe :)
					std::size_t sizeRead = 0;
					while(sizeRead != bufferSize) {
						// Find the size part first
						uint8_t sizeOfControllerData = bufferPointer[sizeRead];
						// Possibility that I will save filespace by making sizeOfControllerData==0 be an empty controller data
						sizeRead += sizeof(sizeOfControllerData);
						// Load the data
						std::shared_ptr<ControllerData> controllerData = std::make_shared<ControllerData>();

						serializeProtocol.binaryToData<ControllerData>(*controllerData, &bufferPointer[sizeRead], sizeOfControllerData);
						// For now, just add each frame one at a time, no optimization
						inputs->push_back(controllerData);
						sizeRead += sizeOfControllerData;
					}

					savestateHook->inputs.push_back(inputs);
				}
			}

			std::ifstream dhashFile(projectDir.GetPathWithSep().ToStdString() + std::string(savestate["dHash"].GetString()));
			savestateHook->dHash = std::string((std::istreambuf_iterator<char>(dhashFile)), (std::istreambuf_iterator<char>()));

			wxImage screenshotImage(projectDir.GetPathWithSep() + wxString::FromUTF8(savestate["screenshot"].GetString()), wxBITMAP_TYPE_JPEG);
			savestateHook->screenshot = new wxBitmap(screenshotImage);

			(*savestateHookBlocks)[savestateHookIndex] = savestateHook;

			savestateHookIndex++;
		}

		players[playerIndex] = savestateHookBlocks;

		playerIndex++;
	}

	// Set dataProcessing
	dataProcessing->setAllPlayers(players);
	dataProcessing->sendPlayerNum();
	dataProcessing->scrollToSpecific(jsonSettings["currentPlayer"].GetUint(), jsonSettings["currentSavestateBlock"].GetUint(), jsonSettings["currentBranch"].GetUint(), jsonSettings["currentFrame"].GetUint64());

	imageExportIndex = jsonSettings["currentImageExportIndex"].GetUint();

	lastEnteredFtpPath = std::string(jsonSettings["defaultFtpPathForExport"].GetString());

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
}

void ProjectHandler::saveProject() {
	// TODO save project every so often
	if(projectWasLoaded) {
		// Save each set of data one by one

		rapidjson::Document settingsJSON;
		settingsJSON.SetObject();

		rapidjson::Value playersJSON(rapidjson::kArrayType);

		AllPlayers& players = dataProcessing->getAllPlayers();

		uint8_t playerIndexNum = 0;
		for(auto const& player : players) {
			AllSavestateHookBlocks& savestateHookBlocks = *player;

			rapidjson::Value savestateHooksJSON(rapidjson::kArrayType);
			SavestateBlockNum savestateHookIndexNum = 0;
			for(auto const& savestateHookBlock : savestateHookBlocks) {
				rapidjson::Value branchesJSON(rapidjson::kArrayType);

				BranchNum branchIndexNum = 0;
				for(auto const& branch : savestateHookBlock->inputs) {
					// Create path as "hooks/player_[num]/savestate_block_[num]/branch_[num]"
					// Will also put dHash here as txt file
					wxFileName inputsFilename = getProjectStart();
					inputsFilename.AppendDir("hooks");
					inputsFilename.AppendDir(wxString::Format("player_%u", playerIndexNum));
					inputsFilename.AppendDir(wxString::Format("savestate_block_%hu", savestateHookIndexNum));
					if(branchIndexNum == 0) {
						inputsFilename.AppendDir("branch_main");
					} else {
						inputsFilename.AppendDir(wxString::Format("branch_%hu", branchIndexNum));
					}
					inputsFilename.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
					inputsFilename.SetName("inputs");
					inputsFilename.SetExt("bin");

					// Delete file if already present and use binary mode
					wxFFileOutputStream inputsFileStream(inputsFilename.GetFullPath(), "wb");
					wxZlibOutputStream inputsCompressStream(inputsFileStream, compressionLevel, wxZLIB_ZLIB);

					// Kinda annoying, but actually break up the vector and add each part with the size
					for(auto const& controllerData : *branch) {
						uint8_t* data;
						uint32_t dataSize;
						serializeProtocol.dataToBinary<ControllerData>(*controllerData, &data, &dataSize);
						uint8_t sizeToPrint = (uint8_t)dataSize;
						// Probably endian issues
						inputsCompressStream.WriteAll(&sizeToPrint, sizeof(sizeToPrint));
						inputsCompressStream.WriteAll(data, dataSize);
					}

					inputsCompressStream.Sync();
					inputsCompressStream.Close();
					inputsFileStream.Close();

					rapidjson::Value branchJSON(rapidjson::kObjectType);
					inputsFilename.MakeRelativeTo(getProjectStart().GetFullPath());

					rapidjson::Value inputs;
					wxString inputsPath = inputsFilename.GetFullPath(wxPATH_UNIX);
					inputs.SetString(inputsPath.c_str(), inputsPath.size(), settingsJSON.GetAllocator());

					branchJSON.AddMember("filename", inputs, settingsJSON.GetAllocator());

					branchesJSON.PushBack(branchJSON, settingsJSON.GetAllocator());

					branchIndexNum++;
				}

				wxFileName dhashFilename = getProjectStart();
				dhashFilename.AppendDir("hooks");
				dhashFilename.AppendDir(wxString::Format("player_%u", playerIndexNum));
				dhashFilename.AppendDir(wxString::Format("savestate_block_%hu", savestateHookIndexNum));
				dhashFilename.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
				dhashFilename.SetName("dhash");
				dhashFilename.SetExt("txt");

				wxFFileOutputStream dhashFile(dhashFilename.GetFullPath(), "w");
				dhashFile.WriteAll(wxString::FromUTF8(savestateHookBlock->dHash), savestateHookBlock->dHash.size());
				dhashFile.Close();

				wxFileName screenshotFileName = dataProcessing->getFramebufferPathForSavestateHook(savestateHookIndexNum);

				savestateHookBlock->screenshot->SaveFile(screenshotFileName.GetFullPath(), wxBITMAP_TYPE_JPEG);

				// Add the item in the savestateHooks JSON
				rapidjson::Value savestateHookJSON(rapidjson::kObjectType);
				screenshotFileName.MakeRelativeTo(getProjectStart().GetFullPath());
				dhashFilename.MakeRelativeTo(getProjectStart().GetFullPath());

				rapidjson::Value dHash;
				wxString dhashPath = dhashFilename.GetFullPath(wxPATH_UNIX);
				dHash.SetString(dhashPath.c_str(), dhashPath.size(), settingsJSON.GetAllocator());

				rapidjson::Value screenshot;
				wxString screenshotPath = screenshotFileName.GetFullPath(wxPATH_UNIX);
				screenshot.SetString(screenshotPath.c_str(), screenshotPath.size(), settingsJSON.GetAllocator());

				savestateHookJSON.AddMember("dHash", dHash, settingsJSON.GetAllocator());
				savestateHookJSON.AddMember("screenshot", screenshot, settingsJSON.GetAllocator());
				savestateHookJSON.AddMember("branches", branchesJSON, settingsJSON.GetAllocator());

				savestateHooksJSON.PushBack(savestateHookJSON, settingsJSON.GetAllocator());

				savestateHookIndexNum++;
			}

			rapidjson::Value playerJSON(rapidjson::kObjectType);

			playerJSON.AddMember("savestateBlocks", savestateHooksJSON, settingsJSON.GetAllocator());

			playersJSON.PushBack(playerJSON, settingsJSON.GetAllocator());

			playerIndexNum++;
		}

		settingsJSON.AddMember("players", playersJSON, settingsJSON.GetAllocator());

		rapidjson::Value lastPlayerIndex;
		lastPlayerIndex.SetUint(dataProcessing->getCurrentPlayer());

		rapidjson::Value lastSavestateHookIndex;
		lastSavestateHookIndex.SetUint(dataProcessing->getCurrentSavestateHook());

		rapidjson::Value lastExportImageIndex;
		lastExportImageIndex.SetUint(imageExportIndex);

		rapidjson::Value lastBranch;
		lastBranch.SetUint64(dataProcessing->getCurrentBranch());

		rapidjson::Value lastFrame;
		lastFrame.SetUint64(dataProcessing->getCurrentFrame());

		settingsJSON.AddMember("currentPlayer", lastPlayerIndex, settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentSavestateBlock", lastSavestateHookIndex, settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentBranch", lastBranch, settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentFrame", lastFrame, settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentImageExportIndex", lastExportImageIndex, settingsJSON.GetAllocator());

		rapidjson::Value defaultFtpPathForExport;
		defaultFtpPathForExport.SetString(lastEnteredFtpPath.c_str(), lastEnteredFtpPath.size(), settingsJSON.GetAllocator());

		settingsJSON.AddMember("defaultFtpPathForExport", defaultFtpPathForExport, settingsJSON.GetAllocator());

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

		// Add to recent projects list if not yet there, otherwise modify
		if(recentProjectChoice == -1) {
			// Add new
			rapidjson::Value newRecentProject(rapidjson::kObjectType);

			rapidjson::Value name;
			name.SetString(projectName.c_str(), strlen(projectName.c_str()), recentSettings.GetAllocator());

			rapidjson::Value directory;
			wxString dirString = projectDir.GetPathWithSep();
			directory.SetString(dirString.mb_str(), dirString.length(), recentSettings.GetAllocator());

			newRecentProject.AddMember("projectDirectory", directory, recentSettings.GetAllocator());
			newRecentProject.AddMember("projectName", name, recentSettings.GetAllocator());

			// I think it's a reference, not sure
			getRecentProjects().PushBack(newRecentProject, recentSettings.GetAllocator());
			recentProjectChoice = getRecentProjects().Size() - 1;
		} else {
			// Modify existing values
			wxString dirString = projectDir.GetPathWithSep();
			getRecentProjects()[recentProjectChoice]["projectDirectory"].SetString(dirString.c_str(), dirString.length(), recentSettings.GetAllocator());

			getRecentProjects()[recentProjectChoice]["projectName"].SetString(projectName.c_str(), projectName.size(), recentSettings.GetAllocator());
		}

		// Additionally, save the mainSettings and overwrite
		// wxFFileOutputStream settingsFileStream(HELPERS::getMainSettingsPath("switas_settings").GetFullPath(), "w");

		// rapidjson::StringBuffer sb;
		// rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		// writer.SetIndent('\t', 1);
		// mainSettings->Accept(writer);

		// settingsFileStream.WriteAll(sb.GetString(), sb.GetLength());
		// settingsFileStream.Close();

		wxFFileOutputStream recentFileStream(HELPERS::getMainSettingsPath("switas_recent").GetFullPath(), "w");

		rapidjson::StringBuffer sbRecent;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writerRecent(sbRecent);
		writerRecent.SetIndent('\t', 1);
		recentSettings.Accept(writerRecent);

		recentFileStream.WriteAll(sbRecent.GetString(), sbRecent.GetLength());
		recentFileStream.Close();
	}
}

void ProjectHandler::newProjectWasCreated() {
	// For now
	dataProcessing->sendPlayerNum();
}

void ProjectHandler::openUpVideoComparisonViewer(int index) {
	wxString projDir              = projectDir.GetName();
	VideoComparisonViewer* viewer = new VideoComparisonViewer(parentFrame, std::bind(&ProjectHandler::closeVideoComparisonViewer, this, std::placeholders::_1), &recentSettings, videoComparisonEntries, projDir);
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

void ProjectHandler::updateVideoComparisonViewers(int delta) {
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
	dir.AppendDir(wxString::Format("TAS_PROJECT_%" wxLongLongFmtSpec, wxGetLocalTimeMillis().GetValue()));
	projectHandler->setProjectDir(dir.GetFullPath());
}

ProjectHandlerWindow::ProjectHandlerWindow(wxFrame* parent, std::shared_ptr<ProjectHandler> projHandler, rapidjson::Document* settings)
	: wxDialog(parent, wxID_ANY, "Select Project", wxDefaultPosition, wxDefaultSize) {
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
	rapidjson::GenericArray<false, rapidjson::Value> recentProjectsArray = projectHandler->getRecentProjects();
	int selectedProject                                                  = event.GetInt();
	if(selectedProject == recentProjectsArray.Size()) {
		// Open up a load project dialog
		wxDirDialog dlg(NULL, "Choose Project Directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
		if(dlg.ShowModal() == wxID_OK && wxFileName(dlg.GetPath()).DirExists()) {
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
		if(dlg.ShowModal() == wxID_OK && wxFileName(dlg.GetPath()).DirExists()) {
			projectHandler->setProjectDir(dlg.GetPath());
			projectHandler->setRecentProjectChoice(-1);
			projectHandler->setProjectName("Unnamed");
			projectHandler->newProjectWasCreated();
			projectChosen       = true;
			wasClosedForcefully = false;

			isNewProject = true;

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
}