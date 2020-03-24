#include "savestateSelection.hpp"

SavestateSelection::SavestateSelection(rapidjson::Document* settings, bool isSavestateLoadDialog)
	: wxDialog(NULL, wxID_ANY, "Savestate Selection", wxDefaultPosition, wxDefaultSize) {
	// Parent is specifically null because this is a separate window that opens
	savestateLoadDialog = isSavestateLoadDialog;
	mainSettings        = settings;

	imageSizer  = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	fullSizer   = new wxBoxSizer(wxVERTICAL);

	// Fill sizers
	// Have to have play, pause and frame advance buttons
	// Look to mainWindow for setting sizers
	// Gonna go a roundabout way for adding buttons, every button will be visible, but certain ones will be disabled as needed

	playButton         = HELPERS::getBitmapButton(this, mainSettings, "playButton");
	pauseButton        = HELPERS::getBitmapButton(this, mainSettings, "pauseButton");
	frameAdvanceButton = HELPERS::getBitmapButton(this, mainSettings, "frameAdvanceButton");
	okButton           = HELPERS::getBitmapButton(this, mainSettings, "okButton");

	playButton->Bind(wxEVT_BUTTON, &SavestateSelection::onPlay, this);
	pauseButton->Bind(wxEVT_BUTTON, &SavestateSelection::onPause, this);
	frameAdvanceButton->Bind(wxEVT_BUTTON, &SavestateSelection::onFrameAdvance, this);
	okButton->Bind(wxEVT_BUTTON, &SavestateSelection::onOk, this);

	currentFrame = new DrawingCanvasBitmap(this, wxSize(1280, 720));
	if(savestateLoadDialog) {
		// Otherwise, just show the frame to save on
		goalFrame = new DrawingCanvasBitmap(this, wxSize(1280, 720));
	}

	imageSizer->Add(currentFrame, 0, wxSHAPED | wxEXPAND);
	if(savestateLoadDialog) {
		imageSizer->Add(goalFrame, 0, wxSHAPED | wxEXPAND);
	}

	buttonSizer->Add(playButton, 1);
	buttonSizer->Add(pauseButton, 1);
	buttonSizer->Add(frameAdvanceButton, 1);
	buttonSizer->Add(okButton, 1);

	fullSizer->Add(imageSizer, 1, wxEXPAND | wxALL);
	fullSizer->Add(buttonSizer, 1, wxEXPAND | wxALL);

	SetSizer(fullSizer);
	fullSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

// clang-format off
BEGIN_EVENT_TABLE(SavestateSelection, wxDialog)
    EVT_IDLE(SavestateSelection::onIdle)
END_EVENT_TABLE()
// clang-format on

void SavestateSelection::onIdle(wxIdleEvent& event) {
	// Read network, TODO
	if(IsShown()) {
	}
}

void SavestateSelection::onPlay(wxCommandEvent& event) {}
void SavestateSelection::onPause(wxCommandEvent& event) {}
void SavestateSelection::onFrameAdvance(wxCommandEvent& event) {}
void SavestateSelection::onOk(wxCommandEvent& event) {}