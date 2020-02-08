#include "sideUI.hpp"
#include <memory>

void FrameCanvas::draw(wxDC* dc) {
	// Do thing
};

SideUI::SideUI(wxPanel* parentFrame, rapidjson::Document* settings, wxBoxSizer* sizer, std::shared_ptr<DataProcessing> input) {
	mainSettings = settings;
	inputData    = input;

	verticalBoxSizer = std::make_shared<wxBoxSizer>(wxVERTICAL);

	// Holds input stuff
	inputsViewSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);

	frameDrawer = std::make_shared<FrameCanvas>();
	frameDrawer->setBackgroundColor(*wxWHITE);

	playBitmap         = std::make_shared<wxBitmap>(HELPERS::resolvePath((*mainSettings)["ui"]["playButton"].GetString()), wxBITMAP_TYPE_PNG);
	frameAdvanceBitmap = std::make_shared<wxBitmap>(HELPERS::resolvePath((*mainSettings)["ui"]["frameAdvanceButton"].GetString()), wxBITMAP_TYPE_PNG);

	playButton = std::make_shared<wxBitmapButton>();
	playButton->SetBitmapCurrent(*playBitmap);
	frameAdvanceButton = std::make_shared<wxBitmapButton>();
	frameAdvanceButton->SetBitmapCurrent(*frameAdvanceBitmap);

	// Button handlers
	playButton->Bind(wxEVT_BUTTON, &SideUI::onPlayPressed, this);
	frameAdvanceButton->Bind(wxEVT_BUTTON, &SideUI::onFrameAdvancePressed, this);

	// TODO all these expands and all seem suspect

	verticalBoxSizer->Add(playButton.get(), wxEXPAND | wxALL);
	verticalBoxSizer->Add(frameAdvanceButton.get(), wxEXPAND | wxALL);

	inputsViewSizer->Add(frameDrawer.get(), wxEXPAND | wxALL);
	inputsViewSizer->Add(inputData.get(), wxEXPAND | wxALL);

	inputsViewSizer->SetSizeHints(parentFrame);

	verticalBoxSizer->Add(inputsViewSizer.get(), wxEXPAND | wxALL);

	verticalBoxSizer->SetSizeHints(parentFrame);

	// Just add it
	sizer->Add(verticalBoxSizer.get(), wxEXPAND | wxALL);
}

void SideUI::onPlayPressed(wxCommandEvent& event) {
	// Clicked
}

void SideUI::onFrameAdvancePressed(wxCommandEvent& event) {
	// New frame must be added, will do more later
	inputData->addNewFrame();
}