#include "runFinalTas.hpp"

TasRunner::TasRunner(wxFrame* parent, std::shared_ptr<CommunicateWithNetwork> networkImp, rapidjson::Document* settings, DataProcessing* inputData)
	: wxDialog(parent, wxID_ANY, "Run Final TAS", wxDefaultPosition, wxDefaultSize) {
	networkInstance = networkImp;
	mainSettings    = settings;
	dataProcessing  = inputData;

	mainSizer          = new wxBoxSizer(wxVERTICAL);
	hookSelectionSizer = new wxBoxSizer(wxHORIZONTAL);

	int lastSavestateHookNum = (int)dataProcessing->getNumOfSavestateHooks(0) - 1;
	firstSavestateHook       = new wxSpinCtrl(parent, wxID_ANY, "First Savestate Hook", wxDefaultPosition, wxDefaultSize, wxSP_WRAP, 0, lastSavestateHookNum, 0);
	lastSavestateHook        = new wxSpinCtrl(parent, wxID_ANY, "Last Savestate Hook", wxDefaultPosition, wxDefaultSize, wxSP_WRAP, 0, lastSavestateHookNum, lastSavestateHookNum);

	firstSavestateHook->SetToolTip("Select first savestate hook (inclusive)");
	lastSavestateHook->SetToolTip("Select last savestate hook (inclusive)");

	hookSelectionSizer->Add(firstSavestateHook, 1, wxEXPAND | wxALL);
	hookSelectionSizer->Add(lastSavestateHook, 1, wxEXPAND | wxALL);

	startTasHomebrew = HELPERS::getBitmapButton(parent, mainSettings, "startTasHomebrewButton");
	startTasArduino  = HELPERS::getBitmapButton(parent, mainSettings, "startTasArduinoButton");

	stopTas = HELPERS::getBitmapButton(parent, mainSettings, "stopButton");

	startTasHomebrew->SetToolTip("Start TAS via homebrew");
	startTasArduino->SetToolTip("Start TAS via arduino");
	stopTas->SetToolTip("Set player");

	startTasHomebrew->Bind(wxEVT_BUTTON, &TasRunner::onStartTasHomebrewPressed, this);
	startTasArduino->Bind(wxEVT_BUTTON, &TasRunner::onStartTasArduinoPressed, this);
	stopTas->Bind(wxEVT_BUTTON, &TasRunner::onStopTasPressed, this);

	mainSizer->Add(hookSelectionSizer, 1, wxEXPAND | wxALL);
	mainSizer->Add(startTasHomebrew, 1, wxEXPAND | wxALL);
	mainSizer->Add(startTasArduino, 1, wxEXPAND | wxALL);
	mainSizer->Add(stopTas, 1, wxEXPAND | wxALL);

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
		// Build a large binary blob with all the data
		AllPlayers& allPlayers = dataProcessing->getAllPlayers();
		// Create a different file for each player
		std::vector<wxString> playerFiles;
		uint8_t playerIndex = 1;
		for(auto const& player : allPlayers) {
			wxString tempPath = wxFileName::CreateTempFileName("script");
			wxFFileOutputStream fileStream(tempPath, "wb");

			for(SavestateBlockNum hook = firstHook; hook <= lastHook; hook++) {
				// Always first branch
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

		uint8_t addressGood = false;
		wxString address;

		if(networkInstance->isConnected()) {
			// Special case, ask again for custom if this fails for some reason
			address     = wxString::FromUTF8(networkInstance->getSwitchIP());
			addressGood = true;
		}

		std::vector<std::string> scriptPaths;

		uint8_t currentWorkingPlayer = 0;
		while(true) {
			if(currentWorkingPlayer == playerFiles.size()) {
				// All players have been successfully dealt with, break the loop
				break;
			}

			if(!addressGood) {
				// Ask for it from the user
				wxString ipAddress = wxGetTextFromUser("Please enter IP address of Nintendo Switch", "Server connect", wxEmptyString);
				if(!ipAddress.empty()) {
					address     = ipAddress;
					addressGood = true;
				} else {
					// If canceled, just forget this and roll back
					for(auto const& playerFile : playerFiles) {
						wxRemoveFile(playerFile);
					}
					return;
				}
			}

			wxString ftpPath = wxString::Format("/switas-script-temp-%d.txt", currentWorkingPlayer);

			std::string output = HELPERS::exec(wxString::Format("curl -T %s -m 10 --connect-timeout 3 --verbose %s", playerFiles[currentWorkingPlayer], wxString::Format("ftp://%s%s", address, ftpPath)).c_str());

			if(output.find("is not recognized") != std::string::npos || output.find("command not found") != std::string::npos) {
				wxMessageDialog errorDialog(this, "The Curl executable was not found, please install it to PATH right now", "Curl Not Found", wxOK | wxICON_ERROR);
				errorDialog.ShowModal();
				// The user should install CURL while this dialog is open
			} else if(output.find("Closing connection 0") != std::string::npos) {
				std::vector<std::string> lines = HELPERS::splitString(output, '\n');

				wxMessageDialog errorDialog(this, wxString::FromUTF8(lines[lines.size() - 1]), "Curl Error", wxOK | wxICON_ERROR);
				errorDialog.ShowModal();

				// The address appears to be faulty, ask the user to investigate
				addressGood = false;
			} else {
				// Successful writing, go on
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