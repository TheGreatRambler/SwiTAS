#include "savestateSelection.hpp"

SavestateLister::SavestateLister(wxFrame* parent, DataProcessing* input)
	: wxDialog(parent, wxID_ANY, "Savestate Listing", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE) {
	inputInstance = input;

	mainSizer = new wxBoxSizer(wxVERTICAL);

	// 3 columns
	projectList       = new wxWrapSizer(wxVERTICAL);
	projectListHolder = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	// Add each individual savestate hook for viewing
	SavestateBlockNum block = 0;
	for(auto const& savestateHook : inputInstance->getAllSavestateHookBlocks()) {
		wxBoxSizer* itemSizer = new wxBoxSizer(wxVERTICAL);

		// Size is reduced by 4 so the bitmap isn't massive
		DrawingCanvasBitmap* drawingCanvas = new DrawingCanvasBitmap(this, wxSize(1280 / 4, 720 / 4));
		drawingCanvas->setBitmap(savestateHook->screenshot);
		drawingCanvas->SetToolTip(wxString::FromUTF8(savestateHook->dHash));

		savestateScreenshots.push_back(drawingCanvas);

		itemSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format("Savestate Hook %u", block), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 0);
		itemSizer->Add(drawingCanvas, 0, wxSHAPED);

		projectList->Add(itemSizer);

		block++;
	}

	projectListHolder->Bind(wxEVT_LEFT_DOWN, &SavestateLister::onSavestateHookSelect, this);

	projectListHolder->SetSizer(projectList);

	mainSizer->Add(projectListHolder, 1, wxEXPAND | wxALL);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void SavestateLister::onSavestateHookSelect(wxMouseEvent& event) {
	// Get mouse location in scrolled window and compare to each screenshot
	wxPoint positionOnScreen = projectListHolder->ClientToScreen(event.GetPosition());
	for(std::size_t i = 0; i < savestateScreenshots.size(); i++) {
		if(savestateScreenshots[i]->GetScreenRect().Contains(positionOnScreen)) {
			selectedSavestate   = i;
			operationSuccessful = true;
			EndModal(wxID_OK);
			break;
		}
	}
}

SavestateSelection::SavestateSelection(wxFrame* parent, rapidjson::Document* settings, std::shared_ptr<ProjectHandler> projHandler, bool isSavestateLoadDialog, std::shared_ptr<CommunicateWithNetwork> networkImp)
	: wxDialog(parent, wxID_ANY, "Savestate Selection", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxMAXIMIZE) {
	// Parent is specifically null because this is a separate window that opens
	savestateLoadDialog   = isSavestateLoadDialog;
	mainSettings          = settings;
	networkInstance       = networkImp;
	projectHandler        = projHandler;
	autoFrameAdvanceTimer = new wxTimer(this);

	dhashWidth  = (*mainSettings)["dhashWidth"].GetInt();
	dhashHeight = (*mainSettings)["dhashHeight"].GetInt();

	imageSizer  = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	fullSizer   = new wxBoxSizer(wxVERTICAL);

	leftImageSizer = new wxBoxSizer(wxVERTICAL);

	if(savestateLoadDialog) {
		rightImageSizer = new wxBoxSizer(wxVERTICAL);
	}

	if(savestateLoadDialog) {
		leftDHash  = new wxStaticText(this, wxID_ANY, "null", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
		rightDHash = new wxStaticText(this, wxID_ANY, "null", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);

		hammingDistance = new wxStaticText(this, wxID_ANY, "null", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	}

	// To set the text, just use:
	// leftDHash->SetLabelText(aString);

	// Fill sizers
	// Have to have play, pause and frame advance buttons
	// Look to mainWindow for setting sizers
	// Gonna go a roundabout way for adding buttons, every button will be visible, but certain ones will be disabled as needed

	playButton             = HELPERS::getBitmapButton(this, mainSettings, "playButton");
	pauseButton            = HELPERS::getBitmapButton(this, mainSettings, "pauseButton");
	frameAdvanceButton     = HELPERS::getBitmapButton(this, mainSettings, "frameAdvanceButton");
	autoFrameAdvanceButton = HELPERS::getBitmapButton(this, mainSettings, "autoFrameAdvanceButton");
	okButton               = HELPERS::getBitmapButton(this, mainSettings, "okButton");

	playButton->SetToolTip("Start realtime game playback");
	pauseButton->SetToolTip("Pause realtime game playback");
	frameAdvanceButton->SetToolTip("Advance game by one frame");
	autoFrameAdvanceButton->SetToolTip("Start automatic frame advancing");
	okButton->SetToolTip("Select this frame as the savestate hook");

	if(savestateLoadDialog) {
		selectFrameAutomatically = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
		selectFrameAutomatically->SetToolTip("Select frame automatically at or below this hamming distance");
	}

	autoIncrementDelay = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 5000, 0);
	autoIncrementDelay->SetToolTip("Delay in mlliseconds for automatically incrementing frame");

	playButton->Bind(wxEVT_BUTTON, &SavestateSelection::onPlay, this);
	pauseButton->Bind(wxEVT_BUTTON, &SavestateSelection::onPause, this);
	frameAdvanceButton->Bind(wxEVT_BUTTON, &SavestateSelection::onFrameAdvance, this);
	autoFrameAdvanceButton->Bind(wxEVT_BUTTON, &SavestateSelection::onAutoFrameAdvance, this);
	okButton->Bind(wxEVT_BUTTON, &SavestateSelection::onOk, this);

	Bind(wxEVT_TIMER, &SavestateSelection::onAutoFrameAdvanceTimer, this);

	Bind(wxEVT_SIZE, &SavestateSelection::onResize, this);

	// Divide by two because it's usually too big
	currentFrame = new DrawingCanvasBitmap(this, wxSize(1280 / 2, 720 / 2));
	if(savestateLoadDialog) {
		// Otherwise, just show the frame to save on
		goalFrame = new DrawingCanvasBitmap(this, wxSize(1280 / 2, 720 / 2));
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
		fullSizer->Add(hammingDistance, 0, wxEXPAND);
		fullSizer->Add(selectFrameAutomatically, 0, wxEXPAND);
	}

	fullSizer->Add(autoIncrementDelay, 0, wxEXPAND);

	buttonSizer->Add(playButton, 1);
	buttonSizer->Add(pauseButton, 1);
	buttonSizer->Add(frameAdvanceButton, 1);
	buttonSizer->Add(autoFrameAdvanceButton, 1);
	buttonSizer->Add(okButton, 1);

	fullSizer->Add(buttonSizer, 0);

	SetSizer(fullSizer);
	fullSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();

	// Pause at the beginning to get the framebuffer and stuff
	// clang-format off
    ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::PAUSE;
	})
	// clang-format on

	registerFramebufferCallback();
}

// clang-format off
BEGIN_EVENT_TABLE(SavestateSelection, wxDialog)
    EVT_IDLE(SavestateSelection::onIdle)
	EVT_CLOSE(SavestateSelection::onClose)
END_EVENT_TABLE()
// clang-format on

void SavestateSelection::onResize(wxSizeEvent& event) {
	Layout();
	event.Skip();
}

void SavestateSelection::setTargetFrame(wxBitmap* targetBitmap, std::string targetDhash) {
	// Called when it's a load dialog
	goalFrame->setBitmap(targetBitmap);
	rightDHash->SetLabel(wxString::FromUTF8(targetDhash));
}

void SavestateSelection::onAutoFrameAdvanceTimer(wxTimerEvent& event) {
	if(autoFrameEnabled) {
		// Run another frame
		frameAdvance();
	}
}

void SavestateSelection::onIdle(wxIdleEvent& event) {
	PROCESS_NETWORK_CALLBACKS(networkInstance, RecieveGameFramebuffer)
}

void SavestateSelection::registerFramebufferCallback() {
	ADD_NETWORK_CALLBACK(RecieveGameFramebuffer, {
		playButton->Enable();
		frameAdvanceButton->Enable();
		okButton->Enable();

		wxImage screenshot = HELPERS::getImageFromJPEGData(data.buf);
		currentFrame->setBitmap(new wxBitmap(screenshot));
		wxString hash   = HELPERS::calculateDhash(screenshot, dhashWidth, dhashHeight);
		leftDhashString = hash.ToStdString();

		if(savestateLoadDialog) {
			leftDHash->SetLabel(hash);

			uint16_t hamming = HELPERS::getHammingDistance(hash, rightDHash->GetLabel());
			hammingDistance->SetLabel(wxString::Format("%d", hamming));
			if(hamming <= selectFrameAutomatically->GetValue()) {
				// This frame might be identical, ask user if they want to use this frame
				wxMessageDialog useFrameDialog(this, "This frame is very similar to the target frame, use it?", "Use this frame", wxYES_NO | wxCANCEL | wxYES_DEFAULT);
				if(useFrameDialog.ShowModal() == wxID_YES) {
					// Use the frame
					callOk();
				}
			}
		}

		if(autoFrameEnabled) {
			autoFrameAdvanceTimer->StartOnce(autoIncrementDelay->GetValue());
		}
	})
}

void SavestateSelection::onPlay(wxCommandEvent& event) {
	// Trigger automatic playing
	// clang-format off
	ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::UNPAUSE;
	})
	// clang-format on
	paused = false;
}

void SavestateSelection::onPause(wxCommandEvent& event) {
	autoFrameEnabled = false;
	// Stop automatic playing
	// Do the same stuff as frameAdvance
	// clang-format off
	ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::PAUSE;
	})
	// clang-format on
	paused = true;
}

void SavestateSelection::frameAdvance() {
	playButton->Disable();
	frameAdvanceButton->Disable();
	okButton->Disable();

	// clang-format off
	ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::RUN_BLANK_FRAME;
	})
	// clang-format on
}

void SavestateSelection::onFrameAdvance(wxCommandEvent& event) {
	// Send blank input for a single frame then recieve JPEG buffer
	// Blank input in this case is input that matches the inputs of the controller on the switch
	// Set the dHash for the currentFrame and calc the hamming distance
	// Also, disable the window to prevent spam clicking
	if(paused) {
		frameAdvance();
	}
}

void SavestateSelection::onAutoFrameAdvance(wxCommandEvent& event) {
	if(paused) {
		autoFrameEnabled = true;

		frameAdvance();
	}
}

void SavestateSelection::onOk(wxCommandEvent& event) {
	callOk();
}

void SavestateSelection::callOk() {
	// Use this frame as the savestate
	operationSuccessful = true;
	// clang-format off
	ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::START_TAS_MODE;
	})
	// clang-format on
	Close(true);
	EndModal(wxID_OK);
}

void SavestateSelection::onClose(wxCloseEvent& event) {
	delete autoFrameAdvanceTimer;
	projectHandler->Callbacks_RecieveGameFramebuffer.erase(NETWORK_CALLBACK_ID);
}