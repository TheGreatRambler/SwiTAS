#include "runFinalTas.hpp"

TasRunner::TasRunner(wxFrame* parent, std::shared_ptr<CommunicateWithNetwork> networkImp, rapidjson::Document* settings, DataProcessing* inputData)
	: wxDialog(parent, wxID_ANY, "Run Final TAS", wxDefaultPosition, wxSize(600, 400), wxDEFAULT_FRAME_STYLE) {
	networkInstance = networkImp;
	mainSettings    = settings;
	dataProcessing  = inputData;

	mainSizer          = new wxBoxSizer(wxVERTICAL);
	hookSelectionSizer = new wxBoxSizer(wxHORIZONTAL);

	int lastSavestateHookNum = (int)dataProcessing->getNumOfSavestateHooks(0) - 1;
	firstSavestateHook       = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_WRAP, 0, lastSavestateHookNum, 0);
	lastSavestateHook        = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_WRAP, 0, lastSavestateHookNum, lastSavestateHookNum);

	firstSavestateHook->SetToolTip("Select first savestate hook (inclusive)");
	lastSavestateHook->SetToolTip("Select last savestate hook (inclusive)");

	hookSelectionSizer->Add(firstSavestateHook, 1, wxEXPAND | wxALL);
	hookSelectionSizer->Add(lastSavestateHook, 1, wxEXPAND | wxALL);

	consoleLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	consoleLog->Show(false);

	startTasHomebrew = HELPERS::getBitmapButton(this, mainSettings, "startTasHomebrewButton");
	// startTasArduino  = HELPERS::getBitmapButton(this, mainSettings, "startTasArduinoButton");

	stopTas = HELPERS::getBitmapButton(this, mainSettings, "stopButton");

	startTasHomebrew->SetToolTip("Start TAS via homebrew");
	// startTasArduino->SetToolTip("Start TAS via arduino");
	stopTas->SetToolTip("Set player");

	startTasHomebrew->Bind(wxEVT_BUTTON, &TasRunner::onStartTasHomebrewPressed, this);
	// startTasArduino->Bind(wxEVT_BUTTON, &TasRunner::onStartTasArduinoPressed, this);
	stopTas->Bind(wxEVT_BUTTON, &TasRunner::onStopTasPressed, this);

	mainSizer->Add(hookSelectionSizer, 1, wxEXPAND | wxALL);
	mainSizer->Add(consoleLog, 3, wxEXPAND | wxALL);
	mainSizer->Add(startTasHomebrew, 0, wxALIGN_CENTER_HORIZONTAL);
	// mainSizer->Add(startTasArduino, 0, wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(stopTas, 0, wxALIGN_CENTER_HORIZONTAL);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Center(wxBOTH);

	Layout();
}

void TasRunner::onStartTasHomebrewPressed(wxCommandEvent& event) {
	int firstHook = firstSavestateHook->GetValue();
	int lastHook  = lastSavestateHook->GetValue();

	if(lastHook < firstHook) {
		wxMessageDialog invalidRangeMessage(this, "Invalid savestate hook range", "Invalid Range", wxOK | wxICON_ERROR);
		invalidRangeMessage.ShowModal();
	} else {
		if(networkInstance->isConnected()) {
			consoleLog->AppendText("Starting homebrew run final TAS\n");
			// Build a large binary blob with all the data
			AllPlayers& allPlayers                     = dataProcessing->getAllPlayers();
			ExtraFrameDataContainer& allExtraFrameData = dataProcessing->getAllExtraFrameData();
			// Create a different file for each player

			uint8_t playerIndex = 0;
			consoleLog->Show(true);
			Layout();

			std::vector<std::string> scriptPaths;

			for(auto const& player : allPlayers) {
				std::string path = wxString::Format("/switas-script-temp-%d.bin", playerIndex).ToStdString();
				scriptPaths.push_back(path);

				ADD_TO_QUEUE(SendFinalTasChunk, networkInstance, {
					data.openFile  = true;
					data.closeFile = false;
					data.path      = path;
				})

				std::vector<uint8_t> dataToSend;
				dataToSend.clear();

				FrameNum frame = 0;
				for(SavestateBlockNum hook = firstHook; hook <= lastHook; hook++) {
					// Always first branch
					uint64_t frameDelay = player->at(hook)->runFinalTasDelayFrames;
					for(uint64_t i = 0; i < frameDelay; i++) {
						// A size of 0 means no frame
						uint8_t noFrameHere = 0;
						dataToSend.insert(dataToSend.end(), &noFrameHere, &noFrameHere + sizeof(noFrameHere));
					}
					auto& mainBranch    = *(player->at(hook)->inputs[0]);
					FrameNum branchSize = mainBranch.size();
					for(auto const& controllerData : mainBranch) {
						// Continually write the savestate hook data in one unbroken stream
						uint8_t* data;
						uint32_t dataSize;
						serializeProtocol.dataToBinary<ControllerData>(*controllerData, &data, &dataSize);
						uint8_t sizeToPrint = (uint8_t)dataSize;
						// Probably endian issues
						dataToSend.insert(dataToSend.end(), &sizeToPrint, &sizeToPrint + sizeof(sizeToPrint));
						dataToSend.insert(dataToSend.end(), data, data + dataSize);

						if(frame != 0 && (frame % 60 == 0 || frame == (branchSize - 1))) {
							float progress = ((float)frame / branchSize) * 100.0;
							consoleLog->AppendText(wxString::Format("Progress serializing controller data: %.5f%% %lu/%lu, in savestate %u player %u\n", progress, frame, branchSize, hook, playerIndex));
						}

						if(dataToSend.size() > 1000 || frame == (branchSize - 1)) {
							// Dump the data
							ADD_TO_QUEUE(SendFinalTasChunk, networkInstance, {
								data.openFile  = false;
								data.closeFile = false;
								data.path      = path;
								data.contents  = dataToSend;
							})

							consoleLog->AppendText(wxString::Format("Sent controller data %zu bytes long", dataToSend.size()));

							dataToSend.clear();
						}

						frame++;
					}
				}

				ADD_TO_QUEUE(SendFinalTasChunk, networkInstance, {
					data.openFile  = false;
					data.closeFile = true;
					data.path      = path;
				})

				playerIndex++;
			}

			std::string extraDataPath = "/switas-script-extra-temp.bin";

			ADD_TO_QUEUE(SendFinalTasChunk, networkInstance, {
				data.openFile  = true;
				data.closeFile = false;
				data.path      = extraDataPath;
			})

			std::vector<uint8_t> dataToSend;
			dataToSend.clear();

			FrameNum frame = 0;
			for(SavestateBlockNum hook = firstHook; hook <= lastHook; hook++) {
				uint64_t frameDelay = allPlayers[0]->at(hook)->runFinalTasDelayFrames;
				for(uint64_t i = 0; i < frameDelay; i++) {
					// A size of 0 means no frame
					uint8_t noFrameHere = 0;
					dataToSend.insert(dataToSend.end(), &noFrameHere, &noFrameHere + sizeof(noFrameHere));
				}
				auto& mainBranch    = *(allExtraFrameData[hook]->at(0));
				FrameNum branchSize = mainBranch.size();
				for(auto const& extraData : mainBranch) {
					// Continually write the savestate hook data in one unbroken stream
					uint8_t* data;
					uint32_t dataSize;
					serializeProtocol.dataToBinary<TouchAndKeyboardData>(*extraData, &data, &dataSize);
					uint8_t sizeToPrint = (uint8_t)dataSize;
					// Probably endian issues
					dataToSend.insert(dataToSend.end(), &sizeToPrint, &sizeToPrint + sizeof(sizeToPrint));
					dataToSend.insert(dataToSend.end(), data, data + dataSize);

					if(frame != 0 && (frame % 60 == 0 || frame == (branchSize - 1))) {
						float progress = ((float)frame / branchSize) * 100.0;
						consoleLog->AppendText(wxString::Format("Progress serializing extra data: %.5f%% %lu/%lu, in savestate %u player %u\n", progress, frame, branchSize, hook, playerIndex));
					}

					if(dataToSend.size() > 1000 || frame == (branchSize - 1)) {
						// Dump the data
						ADD_TO_QUEUE(SendFinalTasChunk, networkInstance, {
							data.openFile  = false;
							data.closeFile = false;
							data.path      = extraDataPath;
							data.contents  = dataToSend;
						})

						consoleLog->AppendText(wxString::Format("Sent extra data %zu bytes long", dataToSend.size()));

						dataToSend.clear();
					}

					frame++;
				}
			}

			ADD_TO_QUEUE(SendFinalTasChunk, networkInstance, {
				data.openFile  = false;
				data.closeFile = true;
				data.path      = extraDataPath;
			})

			consoleLog->Show(false);
			Layout();

			// clang-format off
			ADD_TO_QUEUE(SendStartFinalTas, networkInstance, {
				data.controllerDataPaths = scriptPaths;
				data.extraDataPath = extraDataPath;
			})
			// clang-format on
		} else {
			// Not connected, cannot run final TAS with homebrew then
			wxMessageDialog connectedDialog(this, "You must connect to your switch in order to run using this method", "Not Connected", wxOK | wxICON_ERROR);
			connectedDialog.ShowModal();
		}
	}
}

void TasRunner::onStartTasArduinoPressed(wxCommandEvent& event) {
	int firstHook = firstSavestateHook->GetValue();
	int lastHook  = lastSavestateHook->GetValue();

	if(lastHook < firstHook) {
		wxMessageDialog invalidRangeMessage(this, "Invalid savestate hook range", "Invalid Range", wxOK | wxICON_ERROR);
		invalidRangeMessage.ShowModal();
	} else {
		// TODO use arduino TAS
		// UNIMPLEMENTED
	}
}

void TasRunner::onStopTasPressed(wxCommandEvent& event) {
	// clang-format off
	ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::STOP_FINAL_TAS;
	})
	// clang-format on
}