#include "sideUI.hpp"
#include <memory>

void FrameCanvas::draw(wxDC* dc) {
	// Do thing
};

SideUI::SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxFlexGridSizer* sizer, std::shared_ptr<DataProcessing> input) {
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

	verticalBoxSizer->Add(playButton.get(), wxEXPAND | wxALL);
	verticalBoxSizer->Add(frameAdvanceButton.get(), wxEXPAND | wxALL);

	inputsViewSizer->Add(frameDrawer.get(), wxEXPAND | wxALL);
	inputsViewSizer->Add(inputData.get(), wxEXPAND | wxALL);

	verticalBoxSizer->Add(inputsViewSizer.get(), wxEXPAND | wxALL);

	// DataProcessing is itself a list control, so add it
	verticalBoxSizer->Add(input.get(), wxEXPAND | wxALL);

	// Just add it
	sizer->Add(verticalBoxSizer.get(), wxEXPAND | wxALL);
}

void SideUI::onPlayPressed(wxCommandEvent& event) {
	// Clicked
}

void SideUI::onFrameAdvancePressed(wxCommandEvent& event) {
	// Clicked
}