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

	// Holds everything
	verticalBoxSizer = new wxBoxSizer(wxVERTICAL);

	// Holds buttons
	buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	// Holds input stuff
	inputsViewSizer = new wxBoxSizer(wxHORIZONTAL);

	frameDrawer = new FrameCanvas(parentFrame, inputData);
	frameDrawer->setBackgroundColor(*wxBLACK);

	addFrameButton            = HELPERS::getBitmapButton(parentFrame, mainSettings, "addFrameButton");
	frameAdvanceButton        = HELPERS::getBitmapButton(parentFrame, mainSettings, "frameAdvanceButton");
	savestateHookCreateButton = HELPERS::getBitmapButton(parentFrame, mainSettings, "savestateHookCreateButton");
	savestateHookLoadButton   = HELPERS::getBitmapButton(parentFrame, mainSettings, "savestateHookLoadButton");
	playerAddButton           = HELPERS::getBitmapButton(parentFrame, mainSettings, "playerAddButton");
	playerRemoveButton        = HELPERS::getBitmapButton(parentFrame, mainSettings, "playerRemoveButton");

	playerSelect = new wxComboBox(parentFrame, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	inputData->setPlayerInfoCallback(std::bind(&SideUI::setPlayerInfo, this, std::placeholders::_1, std::placeholders::_2));

	untether();

	// Button handlers
	addFrameButton->Bind(wxEVT_BUTTON, &SideUI::onAddFramePressed, this);
	frameAdvanceButton->Bind(wxEVT_BUTTON, &SideUI::onFrameAdvancePressed, this);
	savestateHookCreateButton->Bind(wxEVT_BUTTON, &SideUI::onSavestateHookCreatePressed, this);
	savestateHookLoadButton->Bind(wxEVT_BUTTON, &SideUI::onSavestateHookLoadPressed, this);
	playerAddButton->Bind(wxEVT_BUTTON, &SideUI::onPlayerAddPressed, this);
	playerRemoveButton->Bind(wxEVT_BUTTON, &SideUI::onPlayerRemovePressed, this);

	// TODO all these expands and all seem suspect

	buttonSizer->Add(addFrameButton, 1);
	buttonSizer->Add(frameAdvanceButton, 1);
	buttonSizer->Add(savestateHookCreateButton, 1);
	buttonSizer->Add(savestateHookLoadButton, 1);
	buttonSizer->Add(playerAddButton, 1);
	buttonSizer->Add(playerRemoveButton, 1);

	// Not wxEXPAND
	verticalBoxSizer->Add(buttonSizer, 0, wxEXPAND);

	inputsViewSizer->Add(frameDrawer, 1, wxEXPAND | wxALL);

	inputData->SetMinSize(wxSize(0, 0));
	HELPERS::addDarkmodeWindows(inputData);
	inputsViewSizer->Add(inputData, 5, wxEXPAND | wxALL);

	verticalBoxSizer->Add(inputsViewSizer, 1, wxEXPAND | wxALL);

	sizer->Add(verticalBoxSizer, 2, wxEXPAND | wxALL);
}

void SideUI::setPlayerInfo(uint8_t size, uint8_t selected) {
	// Deselect
	playerSelect->SetSelection(wxNOT_FOUND);
	playerSelect->Clear();
	for(uint8_t i = 0; i < size; i++) {
		playerSelect->Append(wxString::Format("Player %d", i + 1));
	}
	setPlayerFromHere = true;
	playerSelect->SetSelection(selected);
	playerSelect->Refresh();
}

void SideUI::playerSelected(wxCommandEvent& event) {
	if(!setPlayerFromHere) {
		inputData->setPlayer(event.GetSelection());
	} else {
		setPlayerFromHere = false;
	}
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