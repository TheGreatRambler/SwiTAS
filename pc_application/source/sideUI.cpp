#include "sideUI.hpp"
#include <memory>

SideUI::SideUI(rapidjson::Document* settings, wxFlexGridSizer* sizer, std::shared_ptr<DataProcessing> input) {
	mainSettings = settings;

	verticalBoxSizer = std::make_shared<wxBoxSizer>();

	wxBitmap play((*mainSettings)["ui"]["playButton"].GetString(), wxBITMAP_TYPE_PNG);
	wxBitmap frameAdvance((*mainSettings)["ui"]["frameAdvanceButton"].GetString(), wxBITMAP_TYPE_PNG);

	playButton         = std::make_shared<wxBitmapButton>(verticalBoxSizer.get(), -1, play, wxDefaultPosition, wxDefaultSize, 0);
	frameAdvanceButton = std::make_shared<wxBitmapButton>(verticalBoxSizer.get(), -1, frameAdvance, wxDefaultPosition, wxDefaultSize, 0);

	verticalBoxSizer->Add(playButton.get(), wxEXPAND | wxALL);
	verticalBoxSizer->Add(frameAdvanceButton.get(), wxEXPAND | wxALL);

	// DataProcessing is itself a list control, so add it
	verticalBoxSizer->Add(input.get(), wxEXPAND | wxALL);
}