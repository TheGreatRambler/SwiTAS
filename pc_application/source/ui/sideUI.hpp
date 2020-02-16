#pragma once

#include <cstdint>
#include <memory>
#include <rapidjson/document.h>
#include <wx/dcbuffer.h>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"
#include "drawingCanvas.hpp"

class FrameCanvas : public DrawingCanvas {
private:
	DataProcessing* inputData;

	uint32_t currentFirst;
	uint32_t currentLast;

public:
	FrameCanvas(wxFrame* parent, DataProcessing* dataProcessing);

	void rangeUpdated(uint32_t first, uint32_t last);

	virtual void draw(wxDC& dc) override;
};

class SideUI {
private:
	rapidjson::Document* mainSettings;

	wxBoxSizer* verticalBoxSizer;

	wxBitmap* playBitmap;
	wxBitmap* frameAdvanceBitmap;

	wxBitmapButton* playButton;
	wxBitmapButton* frameAdvanceButton;

	wxBoxSizer* buttonSizer;

	// Sizer holding the inputs and the fancy viewer next to them
	wxBoxSizer* inputsViewSizer;

	FrameCanvas* frameDrawer;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	DataProcessing* inputData;

	void onPlayPressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxBoxSizer* sizer, DataProcessing* input);
};