#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"

class SideUI {
private:
	rapidjson::Document* mainSettings;

	std::shared_ptr<wxBoxSizer> verticalBoxSizer;

	std::shared_ptr<wxBitmapButton> playButton;
	std::shared_ptr<wxBitmapButton> frameAdvanceButton;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

public:
	SideUI(rapidjson::Document* settings, wxFlexGridSizer* sizer, std::shared_ptr<DataProcessing> input);
};