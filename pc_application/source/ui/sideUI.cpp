#include "sideUI.hpp"
#include <memory>

void FrameCanvas::draw(wxDC* dc) {
	// Do thing
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

	frameDrawer = std::make_shared<FrameCanvas>();
	frameDrawer->setBackgroundColor(*wxBLUE);

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
	inputsViewSizer->Add(inputData.get(), 1, wxEXPAND | wxALL);

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