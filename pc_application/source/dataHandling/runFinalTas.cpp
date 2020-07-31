#include "runFinalTas.hpp"

TasRunner::TasRunner(wxFrame* parent, std::shared_ptr<CommunicateWithNetwork> networkImp, rapidjson::Document* settings, DataProcessing* inputData)
	: wxDialog(parent, wxID_ANY, "Run Final TAS", wxDefaultPosition, wxSize(200, 300)) {
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

	hookSelectionSizer->Add(firstSavestateHook, 0);
	hookSelectionSizer->Add(lastSavestateHook, 0);

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
	mainSizer->Add(startTasHomebrew, 0);
	// mainSizer->Add(startTasArduino, 0);
	mainSizer->Add(stopTas, 0);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
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
			// Build a large binary blob with all the data
			AllPlayers& allPlayers = dataProcessing->getAllPlayers();
			// Create a different file for each player
			std::vector<wxString> playerFiles;
			uint8_t playerIndex = 0;
			for(auto const& player : allPlayers) {
				wxString tempPath = wxFileName::CreateTempFileName("script");
				wxFFileOutputStream fileStream(tempPath, "wb");

				for(SavestateBlockNum hook = firstHook; hook <= lastHook; hook++) {
					// Always first branch
					uint64_t frameDelay = player->at(hook)->runFinalTasDelayFrames;
					for(uint64_t i = 0; i < frameDelay; i++) {
						// A size of 0 means no frame
						uint8_t noFrameHere = 0;
						fileStream.WriteAll(&noFrameHere, sizeof(noFrameHere));
					}
					for(auto const& controllerData : *(player->at(hook)->inputs[0])) {
						// Continually write the savestate hook data in one unbroken stream
						uint8_t* data;
						uint32_t dataSize;
						serializeProtocol.dataToBinary<ControllerData>(*controllerData, &data, &dataSize);
						uint8_t sizeToPrint = (uint8_t)dataSize;
						// Probably endian issues
						fileStream.WriteAll(&sizeToPrint, sizeof(sizeToPrint));
						fileStream.WriteAll(data, dataSize);
					}
				}

				fileStream.Close();

				playerFiles.push_back(tempPath);
				playerIndex++;
			}

			wxString address = wxString::FromUTF8(networkInstance->getSwitchIP());

			std::vector<std::string> scriptPaths;

			for(uint8_t currentWorkingPlayer = 0; currentWorkingPlayer < playerFiles.size(); currentWorkingPlayer++) {
				wxString ftpPath = wxString::Format("/switas-script-temp-%d.txt", currentWorkingPlayer);

				wxString commandString = wxString::Format("curl -T %s -m 10 --connect-timeout 3 --verbose %s", playerFiles[currentWorkingPlayer], wxString::Format("ftp://%s:%d%s", address, SWITCH_FTP_PORT, ftpPath));
				std::string output     = HELPERS::exec(commandString.c_str());

				if(output.find("is not recognized") != std::string::npos || output.find("command not found") != std::string::npos) {
					wxMessageDialog errorDialog(this, "The Curl executable was not found, please install it to PATH right now", "Curl Not Found", wxOK | wxICON_ERROR);
					errorDialog.ShowModal();

					for(auto const& playerFile : playerFiles) {
						wxRemoveFile(playerFile);
					}

					return;
				} else if(output.find("Closing connection 0") != std::string::npos) {
					std::vector<std::string> lines = HELPERS::splitString(output, '\n');

					wxMessageDialog errorDialog(this, wxString::FromUTF8(lines[lines.size() - 1]), "Curl Error", wxOK | wxICON_ERROR);
					errorDialog.ShowModal();

					for(auto const& playerFile : playerFiles) {
						wxRemoveFile(playerFile);
					}

					return;
				} else {
					// Assume successful writing, go on
					wxRemoveFile(playerFiles[currentWorkingPlayer]);
					scriptPaths.push_back(ftpPath.ToStdString());
					currentWorkingPlayer++;
					// EndModal(wxID_OK);
				}
			}

			// clang-format off
			ADD_TO_QUEUE(SendStartFinalTas, networkInstance, {
				data.scriptPaths = scriptPaths;
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