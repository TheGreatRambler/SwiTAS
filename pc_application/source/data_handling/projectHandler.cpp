#include "projectHandler.hpp"

ProjectSettingsWindow::ProjectSettingsWindow(wxFrame* parentFrame, std::shared_ptr<ProjectHandler> projHandler, rapidjson::Document* settings, std::shared_ptr<CommunicateWithNetwork> network)
	: wxFrame(parentFrame, wxID_ANY, "TAS Settings", wxDefaultPosition, wxSize(600, 400), wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT) {
	// Start hidden
	Hide();

	projectHandler  = projHandler;
	mainSettings    = settings;
	networkInstance = network;

	mainSizer = new wxBoxSizer(wxVERTICAL);

	gameNameSizer    = new wxBoxSizer(wxHORIZONTAL);
	gameTitleIdEntry = new wxTextCtrl(this, wxID_ANY, projectHandler->getTitleId(), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_CENTRE);

	gameTitleIdEntry->Bind(wxEVT_TEXT_ENTER, &ProjectSettingsWindow::onTitleIdEntry, this);

	gameName = new wxStaticText(this, wxID_ANY, wxEmptyString);
	if(projectHandler->getTitleId() != wxEmptyString) {
		gameName->SetLabelText(projectHandler->getGameInfoFromTitleId(projectHandler->getTitleId())->GetAttribute("name"));
	}

	gameNameSizer->Add(getLabel("Title ID/Program ID"), 0, wxEXPAND);
	gameNameSizer->Add(gameTitleIdEntry, 0, wxEXPAND);

	isMobileSizer = new wxBoxSizer(wxHORIZONTAL);
	isMobile      = new wxCheckBox(this, wxID_ANY, wxEmptyString);

	isMobile->SetValue(projectHandler->getDocked());

	isMobileSizer->Add(getLabel("Force Game To Mobile"), 0, wxEXPAND);
	isMobileSizer->Add(isMobile, 0, wxEXPAND);

	mainSizer->Add(gameNameSizer, 0);
	mainSizer->Add(gameName, 0, wxEXPAND);
	mainSizer->Add(isMobileSizer, 0);

	Bind(wxEVT_CLOSE_WINDOW, &ProjectSettingsWindow::onClose, this);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void ProjectSettingsWindow::onTitleIdEntry(wxCommandEvent& event) {
	wxString newTitleId = gameTitleIdEntry->GetLineText(0).ToStdString();

	wxXmlNode* info = projectHandler->getGameInfoFromTitleId(newTitleId);

	if(info != NULL) {
		projectHandler->setTitleId(newTitleId);
		gameName->SetLabelText(projectHandler->getGameInfoFromTitleId(newTitleId)->GetAttribute("name"));
	} else {
		gameTitleIdEntry->SetLabelText(projectHandler->getTitleId());

		wxMessageDialog errorDialog(this, "Title ID Not Found", "That Title ID was not found in the database", wxOK | wxICON_ERROR);
		errorDialog.ShowModal();
	}
}

void ProjectSettingsWindow::onChangeDocked(wxCommandEvent& event) {
	projectHandler->setDocked(isMobile->GetValue());

	// TODO send value to switch
}

void ProjectSettingsWindow::onClose(wxCloseEvent& event) {
	Show(false);
}

ProjectHandler::ProjectHandler(wxFrame* parent, DataProcessing* dataProcessingInstance, rapidjson::Document* settings) {
	dataProcessing = dataProcessingInstance;
	mainSettings   = settings;
	parentFrame    = parent;
	// TODO this is simiar to the joysticks submenu
	videoComparisonEntriesMenu = new wxMenu();

	// Initialize it as empty
	lastEnteredFtpPath = "";

	// Get recent settings
	wxString recentSettingsPath = HELPERS::getMainSettingsPath("switas_recent").GetFullPath();
	if(wxFileExists(recentSettingsPath)) {
		recentSettings = HELPERS::getSettingsFile(recentSettingsPath.ToStdString());
	} else {
		// Initialize it from scratch
		recentSettings = HELPERS::getSettingsFromString("{"
														"	'recentProjects': [],"
														"	'recentVideos': []"
														"}");
	}

	gameDatabaseXML.Load(wxString::FromUTF8(HELPERS::resolvePath("NSWreleases.xml")));

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

	size_t const decompressBuffInSize  = ZSTD_DStreamInSize();
	void* const decompressBuffIn       = malloc(decompressBuffInSize);
	size_t const decompressBuffOutSize = ZSTD_DStreamOutSize();
	void* const decompressBuffOut      = malloc(decompressBuffOutSize);

	ZSTD_DCtx* const dctx = ZSTD_createDCtx();

	auto extraFrameDataArray = jsonSettings["extraFrameData"].GetArray();
	ExtraFrameDataContainer extraFrameDatas(extraFrameDataArray.Size());

	SavestateBlockNum savestateHookIndex = 0;
	for(auto const& savestate : extraFrameDataArray) {
		auto branchesArray = savestate["branches"].GetArray();

		// Not actually a savestate hook, just the vector of branches
		std::shared_ptr<std::vector<std::shared_ptr<std::vector<ExtraFrameData>>>> savestateHook = std::make_shared<std::vector<std::shared_ptr<std::vector<ExtraFrameData>>>>();

		for(auto const& branch : branchesArray) {
			wxString path = projectDir.GetPathWithSep() + wxString::FromUTF8(branch["filename"].GetString());
			if(wxFileName(path).FileExists()) {
				wxMemoryBuffer memoryBuffer;
				FILE* fin = fopen(path.c_str(), "rb");

				// https://github.com/facebook/zstd/blob/dev/examples/streaming_decompression.c
				size_t const toRead = decompressBuffInSize;
				size_t read;
				size_t lastRet = 0;
				int isEmpty    = 1;
				while((read = fread(decompressBuffIn, toRead, 1, fin))) {
					isEmpty             = 0;
					ZSTD_inBuffer input = { decompressBuffIn, read, 0 };
					while(input.pos < input.size) {
						ZSTD_outBuffer output = { decompressBuffOut, decompressBuffOutSize, 0 };
						size_t const ret      = ZSTD_decompressStream(dctx, &output, &input);
						memoryBuffer.AppendData(decompressBuffOut, output.pos);
						lastRet = ret;
					}
				}

				fclose(fin);
				ZSTD_DCtx_reset(dctx, ZSTD_reset_session_only);

				uint8_t* bufferPointer = (uint8_t*)memoryBuffer.GetData();
				std::size_t bufferSize = memoryBuffer.GetDataLen();

				ExtraBranchData inputs = std::make_shared<std::vector<std::shared_ptr<TouchAndKeyboardData>>>();

				// Loop through each part and unserialize it
				// This is 0% endian safe :)
				std::size_t sizeRead = 0;
				while(sizeRead != bufferSize) {
					// Find the size part first
					uint8_t sizeOfControllerData = bufferPointer[sizeRead];
					// Possibility that I will save filespace by making sizeOfControllerData==0 be an empty controller data
					sizeRead += sizeof(sizeOfControllerData);
					// Load the data
					std::shared_ptr<TouchAndKeyboardData> extraControllerData = std::make_shared<TouchAndKeyboardData>();

					serializeProtocol.binaryToData<TouchAndKeyboardData>(*extraControllerData, &bufferPointer[sizeRead], sizeOfControllerData);
					// For now, just add each frame one at a time, no optimization
					inputs->push_back(extraControllerData);
					sizeRead += sizeOfControllerData;
				}

				savestateHook->push_back(inputs);
			}
		}

		extraFrameDatas[savestateHookIndex] = savestateHook;

		savestateHookIndex++;
	}

	dataProcessing->setAllExtraData(extraFrameDatas);

	auto playersArray = jsonSettings["players"].GetArray();
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
					wxMemoryBuffer memoryBuffer;
					FILE* fin = fopen(path.c_str(), "rb");

					// https://github.com/facebook/zstd/blob/dev/examples/streaming_decompression.c
					size_t const toRead = decompressBuffInSize;
					size_t read;
					size_t lastRet = 0;
					int isEmpty    = 1;
					while((read = fread(decompressBuffIn, toRead, 1, fin))) {
						isEmpty             = 0;
						ZSTD_inBuffer input = { decompressBuffIn, read, 0 };
						while(input.pos < input.size) {
							ZSTD_outBuffer output = { decompressBuffOut, decompressBuffOutSize, 0 };
							size_t const ret      = ZSTD_decompressStream(dctx, &output, &input);
							memoryBuffer.AppendData(decompressBuffOut, output.pos);
							lastRet = ret;
						}
					}

					fclose(fin);
					ZSTD_DCtx_reset(dctx, ZSTD_reset_session_only);

					uint8_t* bufferPointer = (uint8_t*)memoryBuffer.GetData();
					std::size_t bufferSize = memoryBuffer.GetDataLen();

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

			savestateHook->runFinalTasDelayFrames = savestate["runFinalTasDelayFrames"].GetUint64();

			(*savestateHookBlocks)[savestateHookIndex] = savestateHook;

			savestateHookIndex++;
		}

		players[playerIndex] = savestateHookBlocks;

		playerIndex++;
	}

	ZSTD_freeDCtx(dctx);

	// Set dataProcessing
	dataProcessing->setAllPlayers(players);
	dataProcessing->sendPlayerNum();
	dataProcessing->scrollToSpecific(jsonSettings["currentPlayer"].GetUint(), jsonSettings["currentSavestateBlock"].GetUint(), jsonSettings["currentBranch"].GetUint(), jsonSettings["currentFrame"].GetUint64());

	rerecordCount      = jsonSettings["currentRerecordCount"].GetUint();
	lastEnteredFtpPath = std::string(jsonSettings["defaultFtpPathForExport"].GetString());
	titleID            = wxString::FromUTF8(jsonSettings["titleId"].GetString());
	isMobile           = jsonSettings["isMobile"].GetBool();

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

		size_t const compressBuffOutSize = ZSTD_CStreamOutSize();
		void* const compressBuffOut      = malloc(compressBuffOutSize);

		ZSTD_CCtx* const cctx = ZSTD_createCCtx();
		ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, compressionLevel);

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

					// Kinda annoying, but actually break up the vector and add each part with the size
					for(auto const& controllerData : *branch) {
						// https://github.com/facebook/zstd/blob/dev/examples/streaming_compression.c
						ZSTD_EndDirective const mode = (branch->back() == controllerData) ? ZSTD_e_end : ZSTD_e_continue;

						uint8_t* data;
						uint32_t dataSize;
						serializeProtocol.dataToBinary<ControllerData>(*controllerData, &data, &dataSize);
						uint8_t sizeToPrint = (uint8_t)dataSize;

						std::size_t inputSize = sizeof(sizeToPrint) + dataSize;
						uint8_t input[inputSize];

						memcpy(&input[0], &sizeToPrint, sizeof(sizeToPrint));
						memcpy(&input[sizeof(sizeToPrint)], data, dataSize);

						free(data);

						ZSTD_inBuffer inputBuffer = { input, inputSize, 0 };

						int finished;
						do {
							ZSTD_outBuffer output  = { compressBuffOut, compressBuffOutSize, 0 };
							size_t const remaining = ZSTD_compressStream2(cctx, &output, &inputBuffer, mode);

							inputsFileStream.WriteAll(compressBuffOut, output.pos);

							finished = mode == ZSTD_e_end ? (remaining == 0) : (inputBuffer.pos == inputBuffer.size);
						} while(!finished);
					}

					inputsFileStream.Close();

					ZSTD_CCtx_reset(cctx, ZSTD_reset_session_only);

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

				rapidjson::Value delayFrames;
				delayFrames.SetUint64(savestateHookBlock->runFinalTasDelayFrames);

				savestateHookJSON.AddMember("dHash", dHash, settingsJSON.GetAllocator());
				savestateHookJSON.AddMember("screenshot", screenshot, settingsJSON.GetAllocator());
				savestateHookJSON.AddMember("branches", branchesJSON, settingsJSON.GetAllocator());
				savestateHookJSON.AddMember("runFinalTasDelayFrames", delayFrames, settingsJSON.GetAllocator());

				savestateHooksJSON.PushBack(savestateHookJSON, settingsJSON.GetAllocator());

				savestateHookIndexNum++;
			}

			rapidjson::Value playerJSON(rapidjson::kObjectType);

			playerJSON.AddMember("savestateBlocks", savestateHooksJSON, settingsJSON.GetAllocator());

			playersJSON.PushBack(playerJSON, settingsJSON.GetAllocator());

			playerIndexNum++;
		}

		rapidjson::Value extraFrameDataJSON(rapidjson::kArrayType);

		ExtraFrameDataContainer& allExtraFrameData = dataProcessing->getAllExtraFrameData();
		SavestateBlockNum savestateHookIndexNum    = 0;

		for(auto const& savestateHookBlock : allExtraFrameData) {
			rapidjson::Value branchesJSON(rapidjson::kArrayType);

			BranchNum branchIndexNum = 0;
			for(auto const& branch : *savestateHookBlock) {
				// Create path as "extra_frame_data/savestate_block_[num]/branch_[num]"
				wxFileName inputsFilename = getProjectStart();
				inputsFilename.AppendDir("extra_frame_data");
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

				// Kinda annoying, but actually break up the vector and add each part with the size
				for(auto const& extraFrameData : *branch) {
					// https://github.com/facebook/zstd/blob/dev/examples/streaming_compression.c
					ZSTD_EndDirective const mode = (branch->back() == extraFrameData) ? ZSTD_e_end : ZSTD_e_continue;

					uint8_t* data;
					uint32_t dataSize;
					serializeProtocol.dataToBinary<TouchAndKeyboardData>(*extraFrameData, &data, &dataSize);
					uint8_t sizeToPrint = (uint8_t)dataSize;

					std::size_t inputSize = sizeof(sizeToPrint) + dataSize;
					uint8_t input[inputSize];

					memcpy(&input[0], &sizeToPrint, sizeof(sizeToPrint));
					memcpy(&input[sizeof(sizeToPrint)], data, dataSize);

					free(data);

					ZSTD_inBuffer inputBuffer = { input, inputSize, 0 };

					int finished;
					do {
						ZSTD_outBuffer output  = { compressBuffOut, compressBuffOutSize, 0 };
						size_t const remaining = ZSTD_compressStream2(cctx, &output, &inputBuffer, mode);

						inputsFileStream.WriteAll(compressBuffOut, output.pos);

						finished = mode == ZSTD_e_end ? (remaining == 0) : (inputBuffer.pos == inputBuffer.size);
					} while(!finished);
				}

				ZSTD_CCtx_reset(cctx, ZSTD_reset_session_only);
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

			// Add the item in the savestateHooks JSON
			rapidjson::Value savestateHookJSON(rapidjson::kObjectType);

			savestateHookJSON.AddMember("branches", branchesJSON, settingsJSON.GetAllocator());

			extraFrameDataJSON.PushBack(savestateHookJSON, settingsJSON.GetAllocator());

			savestateHookIndexNum++;
		}

		ZSTD_freeCCtx(cctx);

		settingsJSON.AddMember("players", playersJSON, settingsJSON.GetAllocator());
		settingsJSON.AddMember("extraFrameData", extraFrameDataJSON, settingsJSON.GetAllocator());

		rapidjson::Value defaultFtpPathForExport;
		defaultFtpPathForExport.SetString(lastEnteredFtpPath.c_str(), lastEnteredFtpPath.size(), settingsJSON.GetAllocator());

		rapidjson::Value titleIdElement;
		titleIdElement.SetString(titleID.c_str(), titleID.size(), settingsJSON.GetAllocator());

		settingsJSON.AddMember("currentPlayer", rapidjson::Value((uint64_t)dataProcessing->getCurrentPlayer()), settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentSavestateBlock", rapidjson::Value((uint64_t)dataProcessing->getCurrentSavestateHook()), settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentBranch", rapidjson::Value((uint64_t)dataProcessing->getCurrentBranch()), settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentFrame", rapidjson::Value((uint64_t)dataProcessing->getCurrentFrame()), settingsJSON.GetAllocator());
		settingsJSON.AddMember("currentRerecordCount", rapidjson::Value((uint64_t)rerecordCount), settingsJSON.GetAllocator());
		settingsJSON.AddMember("defaultFtpPathForExport", defaultFtpPathForExport, settingsJSON.GetAllocator());
		settingsJSON.AddMember("titleId", titleIdElement, settingsJSON.GetAllocator());
		settingsJSON.AddMember("isMobile", rapidjson::Value((bool)isMobile), settingsJSON.GetAllocator());

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

bool ProjectHandler::promptForUpdate() {
	// Remove all temp update files
	wxFileName relativeToExecutable(wxStandardPaths::Get().GetExecutablePath());
	relativeToExecutable.RemoveDir(relativeToExecutable.GetDirCount() - 1);

	wxArrayString filesList;
	wxDir().GetAllFiles(relativeToExecutable.GetFullPath(), &filesList);

	for(auto const& file : filesList) {
		// Remove artifacts from last update
		if(file.Find(temporaryUpdateFilesSuffix) != wxNOT_FOUND)
			wxRemoveFile(file);
	}

	const wxString releaseUrl = "https://api.github.com/repos/thegreatrambler/switas/releases/latest";
	wxURL url(releaseUrl);
	if(url.GetError() == wxURL_NOERR) {
		wxString jsonData;
		wxInputStream* in = url.GetInputStream();

		if(in && in->IsOk()) {
			wxStringOutputStream jsonStream(&jsonData);
			in->Read(jsonStream);

			rapidjson::Document releaseInfo = HELPERS::getSettingsFromString(jsonData.ToStdString());
			if(strcmp((*mainSettings)["version"].GetString(), releaseInfo["tag_name"].GetString()) != 0) {
				// The release on Github does not match this version, prompt to download
				wxMessageDialog installDialog(parentFrame, wxString::Format("A new release is available for download:\n\n%s", wxString::FromUTF8(releaseInfo["body"].GetString())), "Install new update", wxYES_NO | wxCANCEL | wxNO_DEFAULT | wxICON_QUESTION);
				int res = installDialog.ShowModal();
				if(res == wxID_YES) {
					// Install the update, closes the app entirely
					int currentIndex         = 0;
					uint8_t haveIndex        = false;
					wxOperatingSystemId osId = wxPlatformInfo::Get().GetOperatingSystemId();
					for(auto const& releaseType : releaseInfo["assets"].GetArray()) {
						switch(osId) {
						case wxOS_WINDOWS:
							if(strcmp(releaseType["name"].GetString(), "switas-windows-64bit.zip") == 0)
								break;
						case wxOS_MAC:
							if(strcmp(releaseType["name"].GetString(), "switas-mac-64bit.app.zip") == 0)
								break;
							break;
						case wxOS_UNIX:
							if(strcmp(releaseType["name"].GetString(), "switas-debian-64bit.deb") == 0)
								break;
							break;
						}
						if(haveIndex)
							break;
						currentIndex++;
					}

					wxURL updateUrl(wxString::FromUTF8(releaseInfo["assets"][currentIndex]["browser_download_url"].GetString()));
					if(updateUrl.GetError() == wxURL_NOERR) {
						wxInputStream* updateIn = url.GetInputStream();

						if(updateIn && updateIn->IsOk()) {
							if(osId == wxOS_UNIX) {
								wxString debPath = wxFileName::CreateTempFileName("switasupdatedeb") + ".deb";
								wxFFileOutputStream file(debPath);
								updateIn->Read(file);
								file.Close();

								// This will only begin once the app successfully shuts down
								wxExecute(wxString::Format("wait %lu && "
														   "sudo apt-get install %s &&"
														   "rm %s",
											  wxGetProcessId(), debPath, debPath),
									wxEXEC_ASYNC | wxEXEC_SHOW_CONSOLE);
							} else {
								wxZipEntry* entry;

								// Dont take control of the pointer
								wxZipInputStream zip(*updateIn);
								while(entry = zip.GetNextEntry(), entry != NULL) {
									// The name is its relative path within the archive
									wxFileName filename(entry->GetName());

									// Only handle files
									if(!filename.IsDir()) {
										// Remove first folder, it's 'release', hope path is relative
										filename.RemoveDir(0);

										wxFileName newFilename(relativeToExecutable.GetPathWithSep() + filename.GetFullPath());
										newFilename.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

										// Cant set executable directly, replace differently
										if(newFilename.GetExt() == "exe" || newFilename.GetExt() == "dll") {
											// Rename existing files
											wxFileName modifiedName = wxFileName(newFilename);
											modifiedName.SetName(newFilename.GetName() + temporaryUpdateFilesSuffix);
											wxRenameFile(newFilename.GetFullPath(), modifiedName.GetFullPath());
										}

										if(newFilename.GetName() == "switas_recent" && newFilename.GetExt() == "json") {
											// Ignored
											delete entry;
											continue;
										}

										zip.OpenEntry(*entry);

										// Write binary might not work, I dunno
										wxFFileOutputStream file(newFilename.GetFullPath(), "wb");

										if(!file) {
											// File not created
											delete entry;
											continue;
										}

										zip.Read(file);

										file.Close();
									}

									delete entry;
								}
							}
						}
						delete updateIn;
						return true;
					} else {
						// Throw URL error
					}
				}
			}
		}
		delete in;
	}
	return false;
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
	splashScreen->setBackgroundColor(*wxWHITE);

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