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

SideUI::SideUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ProjectHandler> projHandler, wxBoxSizer* sizer, DataProcessing* input, std::shared_ptr<CommunicateWithNetwork> networkImp, std::function<void()> runFrameCallback) {
	mainSettings           = settings;
	inputData              = input;
	networkInterface       = networkImp;
	projectHandler         = projHandler;
	parent                 = parentFrame;
	incrementFrameCallback = runFrameCallback;

	// Holds everything
	verticalBoxSizer = new wxBoxSizer(wxVERTICAL);

	// Holds buttons
	buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	// Holds input stuff
	inputsViewSizer = new wxBoxSizer(wxHORIZONTAL);

	frameDrawer = new FrameCanvas(parentFrame, inputData);
	frameDrawer->setBackgroundColor(inputData->GetBackgroundColour());

	frameDrawer->SetToolTip("View selected frame");
	inputData->SetToolTip("Edit frames");

	addFrameButton            = HELPERS::getBitmapButton(parentFrame, mainSettings, "addFrameButton");
	frameAdvanceButton        = HELPERS::getBitmapButton(parentFrame, mainSettings, "frameAdvanceButton");
	savestateHookCreateButton = HELPERS::getBitmapButton(parentFrame, mainSettings, "savestateHookCreateButton");
	savestateHookLoadButton   = HELPERS::getBitmapButton(parentFrame, mainSettings, "savestateHookLoadButton");
	savestateHookModifyButton = HELPERS::getBitmapButton(parentFrame, mainSettings, "savestateHookModifyButton");
	playerAddButton           = HELPERS::getBitmapButton(parentFrame, mainSettings, "playerAddButton");
	playerRemoveButton        = HELPERS::getBitmapButton(parentFrame, mainSettings, "playerRemoveButton");
	branchAddButton           = HELPERS::getBitmapButton(parentFrame, mainSettings, "branchAddButton");
	branchRemoveButton        = HELPERS::getBitmapButton(parentFrame, mainSettings, "branchRemoveButton");

	addFrameButton->SetToolTip("Add frame");
	frameAdvanceButton->SetToolTip("Advance frame");
	savestateHookCreateButton->SetToolTip("Create savestate hook");
	savestateHookLoadButton->SetToolTip("Load savestate hook");
	savestateHookModifyButton->SetToolTip("Modify current savestate hook");
	playerAddButton->SetToolTip("Add player");
	playerRemoveButton->SetToolTip("Remove current player");
	branchAddButton->SetToolTip("Add branch");
	branchRemoveButton->SetToolTip("Remove current branch");

	playerSelect = new wxComboBox(parentFrame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	inputData->setPlayerInfoCallback(std::bind(&SideUI::setPlayerInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	playerSelect->Bind(wxEVT_COMBOBOX, &SideUI::playerSelected, this);
	playerSelect->SetToolTip("Set player");

	branchSelect = new wxComboBox(parentFrame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	inputData->setBranchInfoCallback(std::bind(&SideUI::setBranchInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	branchSelect->Bind(wxEVT_COMBOBOX, &SideUI::branchSelected, this);
	branchSelect->SetToolTip("Set branch");

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
	savestateHookModifyButton->Bind(wxEVT_BUTTON, &SideUI::onSavestateHookModifyPressed, this);
	playerAddButton->Bind(wxEVT_BUTTON, &SideUI::onPlayerAddPressed, this);
	playerRemoveButton->Bind(wxEVT_BUTTON, &SideUI::onPlayerRemovePressed, this);
	branchAddButton->Bind(wxEVT_BUTTON, &SideUI::onBranchAddPressed, this);
	branchRemoveButton->Bind(wxEVT_BUTTON, &SideUI::onBranchRemovePressed, this);

	buttonSizer->Add(addFrameButton, 1);
	buttonSizer->Add(frameAdvanceButton, 1);
	buttonSizer->Add(savestateHookCreateButton, 1);
	buttonSizer->Add(savestateHookLoadButton, 1);
	buttonSizer->Add(savestateHookModifyButton, 1);
	buttonSizer->Add(playerAddButton, 1);
	buttonSizer->Add(playerRemoveButton, 1);
	buttonSizer->Add(branchAddButton, 1);
	buttonSizer->Add(branchRemoveButton, 1);

	verticalBoxSizer->Add(buttonSizer, 0, wxEXPAND);
	verticalBoxSizer->Add(playerSelect, 0, wxEXPAND);
	verticalBoxSizer->Add(branchSelect, 0, wxEXPAND);

	inputData->SetMinSize(wxSize(0, 0));
	inputsViewSizer->Add(frameDrawer, 1, wxEXPAND | wxALL);
	inputsViewSizer->Add(inputData, 5, wxEXPAND | wxALL);

	verticalBoxSizer->Add(inputsViewSizer, 1, wxEXPAND | wxALL);

	autoFrameSizer = new wxBoxSizer(wxHORIZONTAL);

	// TODO add these images
	autoFrameStart = HELPERS::getBitmapButton(parentFrame, mainSettings, "autoFrameAdvanceButton");
	autoFrameEnd   = HELPERS::getBitmapButton(parentFrame, mainSettings, "pauseButton");

	autoFrameStart->SetToolTip("Start auto frame advance");
	autoFrameEnd->SetToolTip("Stop auto frame advance");

	autoFrameStart->Bind(wxEVT_BUTTON, &SideUI::onStartAutoFramePressed, this);
	autoFrameEnd->Bind(wxEVT_BUTTON, &SideUI::onEndAutoFramePressed, this);

	// Name is a misnomer
	autoRunFramesPerSecond = new wxSpinCtrl(parentFrame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 5000, 0);

	autoRunFramesPerSecond->SetToolTip("Delay in mlliseconds for automatically incrementing frame");

	autoRunWithFramebuffer    = new wxCheckBox(parentFrame, wxID_ANY, "Include Screenshot");
	autoRunWithControllerData = new wxCheckBox(parentFrame, wxID_ANY, "Include Controller Data");

	autoRunWithFramebuffer->SetValue(true);
	autoRunWithControllerData->SetValue(true);

	autoFrameSizer->Add(autoFrameStart, 0, wxEXPAND | wxALL);
	autoFrameSizer->Add(autoFrameEnd, 0, wxEXPAND | wxALL);

	verticalBoxSizer->Add(autoFrameSizer, 0, wxEXPAND | wxALL);
	verticalBoxSizer->Add(autoRunFramesPerSecond, 0, wxEXPAND | wxALL);
	verticalBoxSizer->Add(autoRunWithFramebuffer, 0, wxEXPAND | wxALL);
	verticalBoxSizer->Add(autoRunWithControllerData, 0, wxEXPAND | wxALL);

	sizer->Add(verticalBoxSizer, 0, wxEXPAND | wxALL);

	untether();
	setPlayerInfo(1, 0, false);
	setBranchInfo(1, 0, false);
}

void SideUI::handleUnexpectedControllerSize() {
	setPlayerInfo(inputData->getAllPlayers().size(), inputData->getCurrentPlayer(), true);
}

void SideUI::setPlayerInfo(uint8_t size, uint8_t selected, bool force) {
	// Deselect
	playerSelect->SetSelection(wxNOT_FOUND);

	if(playerSelect->GetCount() != size || force) {
		// Size has changed, time to inform the sysmodule
		if(networkInterface->isConnected()) {
			// clang-format off
			ADD_TO_QUEUE(SendSetNumControllers, networkInterface, {
				data.size = size;
			})
			// clang-format on
			// Now, user has to disconnect their controllers and don't allow continuing until done
			while(true) {
				wxMessageDialog removeControllersDialog(parent, "Remove all controllers from the switch, can connect them afterwards", "Remove controllers", wxOK | wxICON_INFORMATION);
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

void SideUI::playerSelected(wxCommandEvent& event) {
	inputData->setPlayer(event.GetSelection());
}

void SideUI::setBranchInfo(uint8_t size, uint8_t selected, bool force) {
	// Deselect
	branchSelect->SetSelection(wxNOT_FOUND);

	if(branchSelect->GetCount() != size || force) {
		branchSelect->Clear();
		branchSelect->Append("Main Branch");
		for(BranchNum i = 1; i < size; i++) {
			branchSelect->Append(wxString::Format("Branch %d", i));
		}
	}

	branchSelect->SetSelection(selected);
	branchSelect->Refresh();
}

void SideUI::branchSelected(wxCommandEvent& event) {
	inputData->setBranch(event.GetSelection());
}

void SideUI::onIdle(wxIdleEvent& event) {
	// Called by MainWindow
	PROCESS_NETWORK_CALLBACKS(networkInterface, RecieveFlag)
}

void SideUI::onAddFramePressed(wxCommandEvent& event) {
	// Add frame
	inputData->addFrameHere();
}

void SideUI::onFrameAdvancePressed(wxCommandEvent& event) {
	// MUST be tethered
	if(tethered) {
		incrementFrameCallback();
		inputData->runFrame(false, false, true);
		disableAdvance();
	}
}

void SideUI::onSavestateHookCreatePressed(wxCommandEvent& event) {
	createSavestateHook();
}

void SideUI::onSavestateHookLoadPressed(wxCommandEvent& event) {
	// Open up the savestate viewer
	SavestateLister savestateSelection(parent, inputData);

	savestateSelection.ShowModal();

	if(savestateSelection.getOperationSuccessful()) {
		loadSavestateHook(savestateSelection.getSelectedSavestate());
	}
}

void SideUI::onSavestateHookModifyPressed(wxCommandEvent& event) {
	if(networkInterface->isConnected()) {
		// Open create dialog but copy the properties
		// Onto the current hook
		SavestateSelection modifySavestateSelection(parent, mainSettings, projectHandler, false, networkInterface);
		modifySavestateSelection.ShowModal();

		if(modifySavestateSelection.getOperationSuccessful()) {
			auto hook = inputData->getAllSavestateHookBlocks()[inputData->getCurrentSavestateHook()];

			if(hook->screenshot != nullptr) {
				delete hook->screenshot;
			}

			hook->dHash      = modifySavestateSelection.getNewDhash();
			hook->screenshot = modifySavestateSelection.getNewScreenshot();

			inputData->invalidateRun(0);

			modifySavestateSelection.getNewScreenshot()->SaveFile(inputData->getFramebufferPathForCurrentFramebuf().GetFullPath(), wxBITMAP_TYPE_JPEG);

			inputData->setSavestateHook(inputData->getCurrentSavestateHook());

			tether();
		} else {
			untether();
		}
	}
}

void SideUI::onPlayerAddPressed(wxCommandEvent& event) {
	inputData->addNewPlayer();
}

void SideUI::onPlayerRemovePressed(wxCommandEvent& event) {
	inputData->removeThisPlayer();
}

void SideUI::onBranchAddPressed(wxCommandEvent& event) {
	inputData->addNewBranch();
}

void SideUI::onBranchRemovePressed(wxCommandEvent& event) {
	inputData->removeThisBranch();
}

bool SideUI::createSavestateHook() {
	// Create savestate and push it onto dataProcessing
	AllSavestateHookBlocks& blocks = inputData->getAllSavestateHookBlocks();
	if(blocks.size() != 1 && blocks[0]->inputs[0]->size() != 1) {
		// Not a new project, add the savestate hook before continuing
		inputData->addNewSavestateHook("", HELPERS::getDefaultSavestateScreenshot());
	}
	// Open up the savestate viewer
	if(networkInterface->isConnected()) {
		SavestateSelection savestateSelection(parent, mainSettings, projectHandler, false, networkInterface);
		savestateSelection.ShowModal();

		if(savestateSelection.getOperationSuccessful()) {
			if(blocks[blocks.size() - 1]->screenshot != nullptr) {
				delete blocks[blocks.size() - 1]->screenshot;
			}

			blocks[blocks.size() - 1]->dHash      = savestateSelection.getNewDhash();
			blocks[blocks.size() - 1]->screenshot = savestateSelection.getNewScreenshot();

			savestateSelection.getNewScreenshot()->SaveFile(inputData->getFramebufferPathForCurrentFramebuf().GetFullPath(), wxBITMAP_TYPE_JPEG);

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

		if(savestateHook->dHash == "") {
			// This is an empty savestate hook made without internet
			// Allow it
			inputData->setSavestateHook(block);
			inputData->sendPlayerNum();
			return true;
		}

		SavestateSelection savestateSelection(parent, mainSettings, projectHandler, true, networkInterface);
		savestateSelection.setTargetFrame(savestateHook->screenshot, savestateHook->dHash);

		savestateSelection.ShowModal();

		if(savestateSelection.getOperationSuccessful()) {
			projectHandler->incrementRerecordCount();
			inputData->setSavestateHook(block);
			// inputData->sendPlayerNum();
			tether();
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
	autoFrameEnd->Enable(false);
	inputData->setTethered(false);
	tethered = false;
}

void SideUI::tether() {
	// wxLogMessage("Tether Switch");
	frameAdvanceButton->Enable(true);
	autoFrameStart->Enable(true);
	autoFrameEnd->Enable(true);
	inputData->setTethered(true);

	tethered = true;
}

void SideUI::onStartAutoFramePressed(wxCommandEvent& event) {
	// autoTimer.Start(1000 / (float)autoRunFramesPerSecond->GetValue(), wxTIMER_CONTINUOUS);
	autoRunActive = true;
	autoFrameStart->Disable();
	sendAutoRunData();
}

void SideUI::sendAutoRunData() {
	if(autoRunActive) {
		if(autoRunWithControllerData->GetValue()) {
			inputData->sendAutoAdvance(autoRunWithFramebuffer->GetValue());
		} else {
			inputData->runFrame(false, false, autoRunWithFramebuffer->GetValue());
		}
	}
}

void SideUI::onEndAutoFramePressed(wxCommandEvent& event) {
	autoRunActive = false;
	autoFrameStart->Enable();
}