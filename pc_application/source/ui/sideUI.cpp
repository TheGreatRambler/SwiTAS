#include "sideUI.hpp"
#include <memory>

FrameCanvas::FrameCanvas(wxFrame* parent, std::shared_ptr<DataProcessing> dataProcessing)
	: DrawingCanvas(parent) {
	currentFirst = 0;
	currentLast  = 0;
	inputData    = dataProcessing;

	inputData->setViewableInputsCallback(std::bind(&FrameCanvas::rangeUpdated, this, std::placeholders::_1, std::placeholders::_2));
}

void FrameCanvas::rangeUpdated(uint32_t first, uint32_t last) {
	if(first != currentFirst || last != currentLast) {
		currentFirst = first;
		currentLast  = last;
		// Refresh now to get another draw
		Refresh();
		Update();
	}
}

void FrameCanvas::draw(wxDC& dc) {
	// Do thing
	int width;
	int height;
	GetSize(&width, &height);

	uint8_t numOfItems = currentLast - currentFirst;

	uint8_t boxHeight = floorf((float)height / numOfItems);

	dc.SetPen(*wxGREEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);

	for(uint8_t i = 0; i < numOfItems; i++) {
		if(i % 2 == 0) {
			dc.SetBrush(*wxLIGHT_GREY_BRUSH);
		} else {
			dc.SetBrush(*wxBLUE_BRUSH);
		}
		dc.DrawRectangle(wxPoint(4, boxHeight * i + 4), wxSize(width - 8, boxHeight - 4));
	}
};

SideUI::SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxBoxSizer* sizer, std::shared_ptr<DataProcessing> input) {
	mainSettings = settings;
	inputData    = input;

	// Holds everything
	verticalBoxSizer = std::make_shared<wxBoxSizer>(wxVERTICAL);

	// Holds buttons
	buttonSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);

	// Holds input stuff
	inputsViewSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);

	frameDrawer = std::make_shared<FrameCanvas>(parentFrame, inputData);
	frameDrawer->setBackgroundColor(*wxLIGHT_GREY);

	wxImage resizedPlayImage(HELPERS::resolvePath((*mainSettings)["ui"]["playButton"].GetString()));
	resizedPlayImage.Rescale((*mainSettings)["ui"]["buttonWidth"].GetInt(), (*mainSettings)["ui"]["buttonHeight"].GetInt());
	wxImage resizedFrameAdvanceImage(HELPERS::resolvePath((*mainSettings)["ui"]["frameAdvanceButton"].GetString()));
	resizedFrameAdvanceImage.Rescale((*mainSettings)["ui"]["buttonWidth"].GetInt(), (*mainSettings)["ui"]["buttonHeight"].GetInt());

	playBitmap         = std::make_shared<wxBitmap>(resizedPlayImage);
	frameAdvanceBitmap = std::make_shared<wxBitmap>(resizedFrameAdvanceImage);

	playButton         = std::make_shared<wxBitmapButton>(parentFrame, wxID_ANY, *playBitmap);
	frameAdvanceButton = std::make_shared<wxBitmapButton>(parentFrame, wxID_ANY, *frameAdvanceBitmap);

	// Button handlers
	playButton->Bind(wxEVT_BUTTON, &SideUI::onPlayPressed, this);
	frameAdvanceButton->Bind(wxEVT_BUTTON, &SideUI::onFrameAdvancePressed, this);

	// TODO all these expands and all seem suspect

	buttonSizer->Add(playButton.get(), 1);
	buttonSizer->Add(frameAdvanceButton.get(), 1);

	// Not wxEXPAND
	verticalBoxSizer->Add(buttonSizer.get(), 0, wxEXPAND);

	inputsViewSizer->Add(frameDrawer.get(), 1, wxEXPAND | wxALL);
	// Dataprocessing is itself a wxListCtrl
	// Setting the minsize so it can get very small
	inputData->SetMinSize(wxSize(0, 0));
	inputsViewSizer->Add(inputData.get(), 5, wxEXPAND | wxALL);

	verticalBoxSizer->Add(inputsViewSizer.get(), 1, wxEXPAND | wxALL);

	// Problems with segfault at close can be traced back to the fact that wxWidgets recieves raw pointers
	// of the sizers, but we own shared pointers, will fix later
	sizer->Add(verticalBoxSizer.get(), 1, wxEXPAND | wxALL);
}

void SideUI::onPlayPressed(wxCommandEvent& event) {
	// Clicked
}

void SideUI::onFrameAdvancePressed(wxCommandEvent& event) {
	// New frame must be added, will do more later
	inputData->addNewFrame();
}