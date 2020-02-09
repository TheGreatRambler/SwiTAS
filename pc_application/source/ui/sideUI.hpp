#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <wx/dcbuffer.h>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"
#include "drawingCanvas.hpp"

class FrameCanvas : public DrawingCanvas {
public:
	void draw(wxDC* dc) override;
};

class SideUI {
private:
	rapidjson::Document* mainSettings;

	std::shared_ptr<wxBoxSizer> verticalBoxSizer;

	std::shared_ptr<wxBitmap> playBitmap;
	std::shared_ptr<wxBitmap> frameAdvanceBitmap;

	std::shared_ptr<wxBitmapButton> playButton;
	std::shared_ptr<wxBitmapButton> frameAdvanceButton;

	// Sizer holding the inputs and the fancy viewer next to them
	std::shared_ptr<wxBoxSizer> inputsViewSizer;

	std::shared_ptr<FrameCanvas> frameDrawer;

	std::shared_ptr<DataProcessing> inputData;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

	void onPlayPressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxBoxSizer* sizer, std::shared_ptr<DataProcessing> input);
};