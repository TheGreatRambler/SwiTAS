#include "sideUI.hpp"

FrameCanvas::FrameCanvas(wxFrame* parent, DataProcessing* dataProcessing)
	: DrawingCanvas(parent, wxDefaultSize) {
	currentFirst = 0;
	currentLast  = 0;
	inputData    = dataProcessing;

	inputData->setViewableInputsCallback(std::bind(&FrameCanvas::rangeUpdated, this, std::placeholders::_1, std::placeholders::_2));
	inputData->setChangingSelectedFrameCallback(std::bind(&FrameCanvas::currentFrameUpdated, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void FrameCanvas::rangeUpdated(FrameNum first, FrameNum last) {
	if(first != currentFirst || last != currentLast) {
		currentFirst = first;
		currentLast  = last;
		// Refresh now to get another draw
		Refresh();
	}
}

void FrameCanvas::currentFrameUpdated(FrameNum frame, FrameNum runFrame, FrameNum imageFrame) {
	currentFrame      = frame;
	currentRunFrame   = runFrame;
	currentImageFrame = imageFrame;
	// Refresh everything, might be overkill
	Refresh();
}

void FrameCanvas::draw(wxDC& dc) {
	// Do thing
	int width;
	int height;
	GetSize(&width, &height);

	uint8_t numOfItems = currentLast - currentFirst + 1;

	// uint8_t boxHeight = floorf((float)height / numOfItems);
	wxRect firstRect = inputData->getFirstItemRect();

	int startY     = firstRect.GetTopLeft().y;
	int itemHeight = firstRect.GetHeight();

	float halfWidth = std::floor((float)width / 2);

	for(FrameNum i = 0; i < numOfItems; i++) {
		FrameNum frame = i + currentFirst;
		dc.SetPen(*wxBLACK_PEN);
		if(frame == currentFrame) {
			// Draw yellow
			dc.SetBrush(*wxYELLOW_BRUSH);
		} else {
			dc.SetBrush(*wxLIGHT_GREY_BRUSH);
		}

		// Slight offset to make asthetically pleasing
		dc.DrawRectangle(wxPoint(4, startY + itemHeight * i + 4), wxSize(width - 8, itemHeight - 4));

		// Draw current run and image frame stuff
		if(frame == currentRunFrame) {
			// Draw green line
			dc.SetPen(*wxGREEN_PEN);
			dc.SetBrush(*wxGREEN_BRUSH);
			dc.DrawRectangle(wxPoint(2, startY + itemHeight * i + 1), wxSize(halfWidth - 4, 2));
		}
		if(frame == currentImageFrame) {
			// Draw orange line
			dc.SetPen(*wxRED_PEN);
			dc.SetBrush(*wxRED_BRUSH);
			dc.DrawRectangle(wxPoint(halfWidth + 2, startY + itemHeight * i + 1), wxSize(halfWidth - 4, 2));
		}
	}
};

SideUI::SideUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ProjectHandler> projHandler, wxBoxSizer* sizer, DataProcessing* input, std::shared_ptr<CommunicateWithNetwork> networkImp) {
	mainSettings     = settings;
	inputData        = input;
	networkInterface = networkImp;
	projectHandler   = projHandler;
	parent           = parentFrame;

	// Holds everything
	verticalBoxSizer = new wxBoxSizer(wxVERTICAL);

	// Holds buttons
	buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	// Holds input stuff
	inputsViewSizer = new wxBoxSizer(wxHORIZONTAL);

	frameDrawer = new FrameCanvas(parentFrame, inputData);
	frameDrawer->setBackgroundColor(*wxBLACK);

	frameDrawer->SetToolTip("View selected frame");
	inputData->SetToolTip("Edit frames");

	addFrameButton            = HELPERS::getBitmapButton(parentFrame, mainSettings, "addFrameButton");
	frameAdvanceButton        = HELPERS::getBitmapButton(parentFrame, mainSettings, "frameAdvanceButton");
	savestateHookCreateButton = HELPERS::getBitmapButton(parentFrame, mainSettings, "savestateHookCreateButton");
	savestateHookLoadButton   = HELPERS::getBitmapButton(parentFrame, mainSettings, "savestateHookLoadButton");
	playerAddButton           = HELPERS::getBitmapButton(parentFrame, mainSettings, "playerAddButton");
	playerRemoveButton        = HELPERS::getBitmapButton(parentFrame, mainSettings, "playerRemoveButton");

	addFrameButton->SetToolTip("Add frame");
	frameAdvanceButton->SetToolTip("Advance frame");
	savestateHookCreateButton->SetToolTip("Create savestate hook");
	savestateHookLoadButton->SetToolTip("Load savestate hook");
	playerAddButton->SetToolTip("Add player");
	playerRemoveButton->SetToolTip("Remove current player");

	playerSelect = new wxComboBox(parentFrame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	inputData->setPlayerInfoCallback(std::bind(&SideUI::setPlayerInfo, this, std::placeholders::_1, std::placeholders::_2));
	playerSelect->Bind(wxEVT_COMBOBOX, &SideUI::playerSelected, this);
	playerSelect->SetToolTip("Set player");

	// clang-format off
	ADD_NETWORK_CALLBACK(RecieveFlag, {
		if (data.actFlag == RecieveInfo::CONTROLLERS_CONNECTED) {
			controllerEventRecieved = true;
		}
	})
	// clang-format on

	// Button handlers
	addFrameButton->Bind(wxEVT_BUTTON, &SideUI::onAddFramePressed, this);
	frameAdvanceButton->Bind(wxEVT_BUTTON, &SideUI::onFrameAdvancePressed, this);
	savestateHookCreateButton->Bind(wxEVT_BUTTON, &SideUI::onSavestateHookCreatePressed, this);
	savestateHookLoadButton->Bind(wxEVT_BUTTON, &SideUI::onSavestateHookLoadPressed, this);
	playerAddButton->Bind(wxEVT_BUTTON, &SideUI::onPlayerAddPressed, this);
	playerRemoveButton->Bind(wxEVT_BUTTON, &SideUI::onPlayerRemovePressed, this);

	buttonSizer->Add(addFrameButton, 1);
	buttonSizer->Add(frameAdvanceButton, 1);
	buttonSizer->Add(savestateHookCreateButton, 1);
	buttonSizer->Add(savestateHookLoadButton, 1);
	buttonSizer->Add(playerAddButton, 1);
	buttonSizer->Add(playerRemoveButton, 1);

	verticalBoxSizer->Add(buttonSizer, 0, wxEXPAND);
	verticalBoxSizer->Add(playerSelect, 0, wxEXPAND);

	inputData->SetMinSize(wxSize(0, 0));
	inputsViewSizer->Add(frameDrawer, 1, wxEXPAND | wxALL);
	inputsViewSizer->Add(inputData, 5, wxEXPAND | wxALL);

	verticalBoxSizer->Add(inputsViewSizer, 1, wxEXPAND | wxALL);

	autoFrameSizer = new wxBoxSizer(wxHORIZONTAL);

	// TODO add these images
	autoFrameStart = HELPERS::getBitmapButton(parentFrame, mainSettings, "autoFrameStartButton");
	autoFrameEnd   = HELPERS::getBitmapButton(parentFrame, mainSettings, "autoFrameEndButton");

	autoFrameStart->SetToolTip("Start auto frame advance");
	autoFrameEnd->SetToolTip("Stop auto frame advance");

	autoFrameStart->Bind(wxEVT_BUTTON, &SideUI::onStartAutoFramePressed, this);
	autoFrameEnd->Bind(wxEVT_BUTTON, &SideUI::onEndAutoFramePressed, this);

	autoRunFramesPerSecond = new wxSpinCtrl(parentFrame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 1);
	autoRunFramesPerSecond->Bind(wxEVT_SPINCTRL, &SideUI::autoRunIntervalChanged, this);

	autoRunFramesPerSecond->SetToolTip("Set frames per second during auto frame advance");

	autoFrameSizer->Add(autoFrameStart, 0, wxEXPAND | wxALL);
	autoFrameSizer->Add(autoFrameEnd, 0, wxEXPAND | wxALL);

	verticalBoxSizer->Add(autoFrameSizer, 0, wxEXPAND | wxALL);
	verticalBoxSizer->Add(autoRunFramesPerSecond, 0, wxEXPAND | wxALL);

	sizer->Add(verticalBoxSizer, 0, wxEXPAND | wxALL);

	untether();
	setPlayerInfo(1, 0);
}

void SideUI::handleUnexpectedControllerSize() {
	setPlayerInfo(inputData->getAllPlayers().size(), inputData->getCurrentPlayer());
}

void SideUI::setPlayerInfo(uint8_t size, uint8_t selected) {
	// Deselect
	playerSelect->SetSelection(wxNOT_FOUND);

	if(playerSelect->GetCount() != size) {
		// Size has changed, time to inform the sysmodule
		if(networkInterface->isConnected()) {
			// clang-format off
			ADD_TO_QUEUE(SendSetNumControllers, networkInterface, {
				data.size = size;
			})
			// clang-format on
			// Now, user has to disconnect their controllers and don't allow continuing until done
			while(true) {
				wxMessageDialog removeControllersDialog(parent, "Remove controllers", "Remove all controllers from the switch, can connect them afterwards", wxOK | wxICON_INFORMATION);
				removeControllersDialog.ShowModal();

				PROCESS_NETWORK_CALLBACKS(networkInterface, RecieveFlag)

				if(controllerEventRecieved) {
					controllerEventRecieved = false;
					break;
				}
			}
		}

		playerSelect->Clear();
		for(uint8_t i = 0; i < size; i++) {
			playerSelect->Append(wxString::Format("Player %d", i + 1));
		}
	}

	playerSelect->SetSelection(selected);
	playerSelect->Refresh();
}

void SideUI::onIdle(wxIdleEvent& event) {
	// Called by MainWindow
	PROCESS_NETWORK_CALLBACKS(networkInterface, RecieveFlag)
}

void SideUI::playerSelected(wxCommandEvent& event) {
	inputData->setPlayer(event.GetSelection());
}

void SideUI::onAddFramePressed(wxCommandEvent& event) {
	// Add frame
	inputData->addFrameHere();
}

void SideUI::onFrameAdvancePressed(wxCommandEvent& event) {
	// MUST be tethered
	if(tethered) {
		inputData->runFrame();
	}
}

void SideUI::onSavestateHookCreatePressed(wxCommandEvent& event) {
	createSavestateHook();
}

void SideUI::onSavestateHookLoadPressed(wxCommandEvent& event) {
	// Open up the savestate viewer
	SavestateLister savestateSelection(inputData);

	savestateSelection.ShowModal();

	if(savestateSelection.getOperationSuccessful()) {
		loadSavestateHook(savestateSelection.getSelectedSavestate());
	}
}

void SideUI::onPlayerAddPressed(wxCommandEvent& event) {
	inputData->addNewPlayer();
}

void SideUI::onPlayerRemovePressed(wxCommandEvent& event) {
	inputData->removeThisPlayer();
}

bool SideUI::createSavestateHook() {
	// Create savestate and push it onto dataProcessing
	AllSavestateHookBlocks& blocks = inputData->getAllSavestateHookBlocks();
	if(blocks.size() != 1 && blocks[0]->inputs->size() != 1) {
		// Not a new project, add the savestate hook before continuing
		inputData->addNewSavestateHook("", new wxBitmap());
	}
	// Open up the savestate viewer
	if(networkInterface->isConnected()) {
		SavestateSelection savestateSelection(mainSettings, projectHandler, false, networkInterface);
		savestateSelection.ShowModal();

		if(savestateSelection.getOperationSuccessful()) {
			blocks[blocks.size() - 1]->dHash      = savestateSelection.getNewDhash();
			blocks[blocks.size() - 1]->screenshot = savestateSelection.getNewScreenshot();

			inputData->setSavestateHook(blocks.size() - 1);

			tether();
			return true;
		} else {
			untether();
			return false;
		}
	} else {
		// No network testing or anything, bitmap is empty in this case
		inputData->setSavestateHook(blocks.size() - 1);
		return true;
	}
}

bool SideUI::loadSavestateHook(int block) {
	if(networkInterface->isConnected()) {
		std::shared_ptr<SavestateHook> savestateHook = inputData->getAllSavestateHookBlocks()[block];

		SavestateSelection savestateSelection(mainSettings, projectHandler, true, networkInterface);
		savestateSelection.setTargetFrame(savestateHook->screenshot, savestateHook->dHash);

		savestateSelection.ShowModal();

		if(savestateSelection.getOperationSuccessful()) {
			inputData->setSavestateHook(block);
			tether();
			inputData->sendPlayerNum();
			return true;
		} else {
			untether();
			return false;
		}
	} else {
		inputData->setSavestateHook(block);
		return true;
	}
}

void SideUI::untether() {
	// Will need more indication
	// TODO have switch itself notify the PC when fishy business is going on
	// So it can untether itself
	// wxLogMessage("Untether Switch");
	frameAdvanceButton->Enable(false);
	autoFrameStart->Enable(false);
	inputData->setTethered(false);
	tethered = false;
}

void SideUI::tether() {
	// wxLogMessage("Tether Switch");
	frameAdvanceButton->Enable(true);
	autoFrameStart->Enable(true);
	inputData->setTethered(true);

	// Convenient place for this, set the player count sysmodule-side
	// When tethering
	int selection = playerSelect->GetSelection();
	if(selection == wxNOT_FOUND) {
		setPlayerInfo(playerSelect->GetCount(), 0);
	} else {
		setPlayerInfo(playerSelect->GetCount(), selection);
	}

	tethered = true;
}

void SideUI::autoRunIntervalChanged(wxSpinEvent& event) {
	/*
	if(autoTimer.IsRunning()) {
		autoTimer.Stop();
		autoTimer.Start(1000 / (float)autoRunFramesPerSecond->GetValue(), wxTIMER_CONTINUOUS);
	}
	*/
	sendAutoRunData();
}

void SideUI::onStartAutoFramePressed(wxCommandEvent& event) {
	// autoTimer.Start(1000 / (float)autoRunFramesPerSecond->GetValue(), wxTIMER_CONTINUOUS);
	sendAutoRunData();
}

void SideUI::sendAutoRunData() {
	ADD_TO_QUEUE(SendAutoRun, networkInterface, {
		data.fps   = autoRunFramesPerSecond->GetValue();
		data.start = true;
	})
}

void SideUI::onEndAutoFramePressed(wxCommandEvent& event) {
	ADD_TO_QUEUE(SendAutoRun, networkInterface, {
		data.fps   = 0;
		data.start = false;
	})
}

void SideUI::recieveAutoRunData(std::shared_ptr<ControllerData> controllerData) {
	inputData->setControllerDataForAutoRun(controllerData);
}