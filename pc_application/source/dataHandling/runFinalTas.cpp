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

	stopTas = HELPERS::getBitmapButton(parent, mainSettings, "stopTasButton");

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
		// Send frames in a agreed-upon amount, likely around 10 seconds of inputs
		// Switch asks for more inputs when it has only 5 seconds left, unless the PC has send a response
		// saying the last of the inputs have been sent
	}
}

void TasRunner::onStartTasArduinoPressed(wxCommandEvent& event) {
	int firstHook = firstSavestateHook->GetValue();
	int lastHook  = lastSavestateHook->GetValue();

	if(lastHook < firstHook) {
		wxMessageDialog invalidRangeMessage(this, "Invalid savestate hook range", "Invalid Range", wxOK | wxICON_ERROR);
		invalidRangeMessage.ShowModal();
	} else {
	}
}

void TasRunner::onStopTasPressed(wxCommandEvent& event) {}