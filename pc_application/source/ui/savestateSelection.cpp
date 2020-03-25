#include "savestateSelection.hpp"

SavestateSelection::SavestateSelection(rapidjson::Document* settings, bool isSavestateLoadDialog)
	: wxDialog(NULL, wxID_ANY, "Savestate Selection", wxDefaultPosition, wxDefaultSize) {
	// Parent is specifically null because this is a separate window that opens
	savestateLoadDialog = isSavestateLoadDialog;
	mainSettings        = settings;

	imageSizer  = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	fullSizer   = new wxBoxSizer(wxVERTICAL);

	leftImageSizer  = new wxBoxSizer(wxVERTICAL);
	rightImageSizer = new wxBoxSizer(wxVERTICAL);

	leftDHash  = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	rightDHash = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);

	hammingDistance = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);

	// To set the text, just use:
	// leftDHash->SetLabelText(aString);

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

	leftImageSizer->Add(currentFrame, 0, wxSHAPED | wxEXPAND);

	if(savestateLoadDialog) {
		// Dhashes are only used in loading, not creating the first one
		leftImageSizer->Add(leftDHash, 1, wxEXPAND | wxALL);

		rightImageSizer->Add(goalFrame, 0, wxSHAPED | wxEXPAND);
		rightImageSizer->Add(rightDHash, 1, wxEXPAND | wxALL);
	}

	imageSizer->Add(leftImageSizer, 1, wxEXPAND | wxALL);
	if(savestateLoadDialog) {
		imageSizer->Add(rightImageSizer, 1, wxEXPAND | wxALL);
	}

	fullSizer->Add(imageSizer, 1, wxEXPAND | wxALL);

	if(savestateLoadDialog) {
		fullSizer->Add(hammingDistance, 1, wxEXPAND | wxALL);
	}

	buttonSizer->Add(playButton, 1);
	buttonSizer->Add(pauseButton, 1);
	buttonSizer->Add(frameAdvanceButton, 1);
	buttonSizer->Add(okButton, 1);

	fullSizer->Add(buttonSizer, 1, wxEXPAND | wxALL);

	SetSizer(fullSizer);
	fullSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();

	// When done with all the stuff, close with Close(true);
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

void SavestateSelection::onPlay(wxCommandEvent& event) {
	// Trigger automatic playing
}
void SavestateSelection::onPause(wxCommandEvent& event) {
	// Stop automatic playing
}
void SavestateSelection::onFrameAdvance(wxCommandEvent& event) {
	// Send blank input for a single frame then recieve JPEG buffer
	// Set the dHash for the currentFrame and calc the hamming distance
}
void SavestateSelection::onOk(wxCommandEvent& event) {
	// Use this frame as the savestate
	operationSuccessful = true;
	Close(true);
}
