#include "sideUI.hpp"
#include <memory>

FrameCanvas::FrameCanvas(wxFrame* parent, DataProcessing* dataProcessing, wxRect firstItemRect)
	: DrawingCanvas(parent, wxDefaultSize) {
	currentFirst = 0;
	currentLast  = 0;
	inputData    = dataProcessing;
	firstRect    = firstItemRect;

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

	dc.SetPen(*wxBLACK_PEN);

	int startY     = firstRect.GetBottomLeft().y;
	int itemHeight = firstRect.GetHeight();

	float halfWidth = std::floor((float)width / 2);

	for(FrameNum i = 0; i < numOfItems; i++) {
		FrameNum frame = i + currentFirst;
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
			dc.SetBrush(*wxGREEN_BRUSH);
			dc.DrawRectangle(wxPoint(2, startY + itemHeight * i + 1), wxSize(halfWidth - 4, 2));
		}
		if(frame == currentImageFrame) {
			// Draw orange line
			dc.SetBrush(*wxRED_BRUSH);
			dc.DrawRectangle(wxPoint(halfWidth + 2, startY + itemHeight * i + 1), wxSize(halfWidth - 4, 2));
		}
	}
};

SideUI::SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxBoxSizer* sizer, DataProcessing* input) {
	mainSettings = settings;
	inputData    = input;

	// Holds everything
	verticalBoxSizer = new wxBoxSizer(wxVERTICAL);

	// Holds buttons
	buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	// Holds input stuff
	inputsViewSizer = new wxBoxSizer(wxHORIZONTAL);

	frameDrawer = new FrameCanvas(parentFrame, inputData, inputData->getFirstItemRect());
	frameDrawer->setBackgroundColor(*wxLIGHT_GREY);

	wxImage resizedaddFrameImage(HELPERS::resolvePath((*mainSettings)["ui"]["addFrameButton"].GetString()));
	resizedaddFrameImage.Rescale((*mainSettings)["ui"]["buttonWidth"].GetInt(), (*mainSettings)["ui"]["buttonHeight"].GetInt());
	wxImage resizedFrameAdvanceImage(HELPERS::resolvePath((*mainSettings)["ui"]["frameAdvanceButton"].GetString()));
	resizedFrameAdvanceImage.Rescale((*mainSettings)["ui"]["buttonWidth"].GetInt(), (*mainSettings)["ui"]["buttonHeight"].GetInt());
	wxImage resizedSavestateHookImage(HELPERS::resolvePath((*mainSettings)["ui"]["savestateHookButton"].GetString()));
	resizedSavestateHookImage.Rescale((*mainSettings)["ui"]["buttonWidth"].GetInt(), (*mainSettings)["ui"]["buttonHeight"].GetInt());

	addFrameBitmap      = new wxBitmap(resizedaddFrameImage);
	frameAdvanceBitmap  = new wxBitmap(resizedFrameAdvanceImage);
	savestateHookBitmap = new wxBitmap(resizedSavestateHookImage);

	addFrameButton      = new wxBitmapButton(parentFrame, wxID_ANY, *addFrameBitmap);
	frameAdvanceButton  = new wxBitmapButton(parentFrame, wxID_ANY, *frameAdvanceBitmap);
	savestateHookButton = new wxBitmapButton(parentFrame, wxID_ANY, *savestateHookBitmap);

	// Button handlers
	addFrameButton->Bind(wxEVT_BUTTON, &SideUI::onAddFramePressed, this);
	frameAdvanceButton->Bind(wxEVT_BUTTON, &SideUI::onFrameAdvancePressed, this);
	savestateHookButton->Bind(wxEVT_BUTTON, &SideUI::onSavestateHookPressed, this);

	// TODO all these expands and all seem suspect

	buttonSizer->Add(addFrameButton, 1);
	buttonSizer->Add(frameAdvanceButton, 1);
	buttonSizer->Add(savestateHookButton, 1);

	// Not wxEXPAND
	verticalBoxSizer->Add(buttonSizer, 0, wxEXPAND);

	inputsViewSizer->Add(frameDrawer, 1, wxEXPAND | wxALL);
	// Dataprocessing is itself a wxListCtrl
	// Setting the minsize so it can get very small
	inputData->SetMinSize(wxSize(0, 0));
	inputsViewSizer->Add(inputData, 5, wxEXPAND | wxALL);

	verticalBoxSizer->Add(inputsViewSizer, 1, wxEXPAND | wxALL);

	sizer->Add(verticalBoxSizer, 2, wxEXPAND | wxALL);
}

void SideUI::onAddFramePressed(wxCommandEvent& event) {
	// Add frame
	inputData->addNewFrame();
}

void SideUI::onFrameAdvancePressed(wxCommandEvent& event) {
	// New frame must be added, will do more later
	inputData->runFrame();
}

void SideUI::onSavestateHookPressed(wxCommandEvent& event) {
	// New frame must be added, will do more later
	inputData->createSavestateHookHere();
}