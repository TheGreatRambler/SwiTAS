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
	Bind(wxEVT_END_PROCESS, &TasRunner::onCommandDone, this);
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
	mainSizer->Add(startTasHomebrew, 0);
	// mainSizer->Add(startTasArduino, 0);
	mainSizer->Add(stopTas, 0);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Center(wxBOTH);

	Layout();
}

// clang-format off
BEGIN_EVENT_TABLE(TasRunner, wxDialog)
	EVT_IDLE(TasRunner::onIdle)
END_EVENT_TABLE()
// clang-format on

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
			AllPlayers& allPlayers = dataProcessing->getAllPlayers();
			// Create a different file for each player
			playerFiles.clear();
			scriptPaths.clear();

			uint8_t playerIndex = 0;
			consoleLog->Show(true);
			Layout();
			for(auto const& player : allPlayers) {
				wxString tempPath = wxFileName::CreateTempFileName("script");
				wxFFileOutputStream fileStream(tempPath, "wb");

				FrameNum frame = 0;
				for(SavestateBlockNum hook = firstHook; hook <= lastHook; hook++) {
					// Always first branch
					uint64_t frameDelay = player->at(hook)->runFinalTasDelayFrames;
					for(uint64_t i = 0; i < frameDelay; i++) {
						// A size of 0 means no frame
						uint8_t noFrameHere = 0;
						fileStream.WriteAll(&noFrameHere, sizeof(noFrameHere));
					}
					auto& mainBranch = *(player->at(hook)->inputs[0]);
					for(auto const& controllerData : mainBranch) {
						// Continually write the savestate hook data in one unbroken stream
						uint8_t* data;
						uint32_t dataSize;
						serializeProtocol.dataToBinary<ControllerData>(*controllerData, &data, &dataSize);
						uint8_t sizeToPrint = (uint8_t)dataSize;
						// Probably endian issues
						fileStream.WriteAll(&sizeToPrint, sizeof(sizeToPrint));
						fileStream.WriteAll(data, dataSize);

						if(frame != 0 && (frame % 60 == 0 || frame == mainBranch.size() - 1)) {
							consoleLog->AppendText(wxString::Format("Progress serializing frames: %.5F%%, %lu/%lu, in savestate %hu player %u\n", ((double)frame / mainBranch.size()) * 100.0, frame, mainBranch.size(), hook, playerIndex));
						}

						frame++;
					}
				}

				fileStream.Close();

				playerFiles.push_back(tempPath);
				playerIndex++;
			}

			currentWorkingPlayer = 0;
			uploadScript();
		} else {
			// Not connected, cannot run final TAS with homebrew then
			wxMessageDialog connectedDialog(this, "You must connect to your switch in order to run using this method", "Not Connected", wxOK | wxICON_ERROR);
			connectedDialog.ShowModal();
		}
	}
}

void TasRunner::uploadScript() {
	if(currentWorkingPlayer < playerFiles.size()) {
		wxString address = wxString::FromUTF8(networkInstance->getSwitchIP());
		ftpPath          = wxString::Format("/switas-script-temp-%d.bin", currentWorkingPlayer);

		wxString commandString = wxString::Format("curl -T %s -m 10 --connect-timeout 3 --verbose %s", playerFiles[currentWorkingPlayer], wxString::Format("ftp://%s:%d%s", address, SWITCH_FTP_PORT, ftpPath));

		// This will run and the progress will be seen in console
		currentRunningCommand = RUNNING_COMMAND::UPLOAD_SCRIPT;
		wxExecute(commandString, wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE, commandProcess);

	} else {
		// All scripts have finished
		consoleLog->Show(false);
		Layout();
		currentRunningCommand = RUNNING_COMMAND::NO_COMMAND;
		delete commandProcess;
		commandProcess = nullptr;
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

void TasRunner::onCommandDone(wxProcessEvent& event) {
	// Assume successful writing, go on
	wxRemoveFile(playerFiles[currentWorkingPlayer]);
	scriptPaths.push_back(ftpPath.ToStdString());

	currentWorkingPlayer++;
	uploadScript();
}

void TasRunner::onIdle(wxIdleEvent& event) {
	// Check command output
	if(currentRunningCommand != RUNNING_COMMAND::NO_COMMAND) {
		wxInputStream* inputStream = commandProcess->GetInputStream();
		if(inputStream->CanRead()) {
			consoleLog->Show(true);
			Layout();

			// https://forums.wxwidgets.org/viewtopic.php?t=24390#p104181

			char buffer[BUFSIZE];
			wxString text;
			inputStream->Read(buffer, BUFSIZE - 1);
			std::size_t count = inputStream->LastRead();
			if(count > 0) {
				text.Append(buffer, count);
			}

			consoleLog->AppendText(text);
		}
	}
}