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

	FrameNum currentFirst;
	FrameNum currentLast;

	wxRect firstRect;

public:
	FrameCanvas(wxFrame* parent, DataProcessing* dataProcessing, wxRect firstItemRect);

	void rangeUpdated(FrameNum first, FrameNum last);

	virtual void draw(wxDC& dc) override;
};

class SideUI {
private:
	rapidjson::Document* mainSettings;

	wxBoxSizer* verticalBoxSizer;

	wxBitmap* addFrameBitmap;
	wxBitmap* frameAdvanceBitmap;
	wxBitmap* savestateHookBitmap;

	wxBitmapButton* addFrameButton;
	wxBitmapButton* frameAdvanceButton;
	wxBitmapButton* savestateHookButton;

	wxBoxSizer* buttonSizer;

	// Sizer holding the inputs and the fancy viewer next to them
	wxBoxSizer* inputsViewSizer;

	FrameCanvas* frameDrawer;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	DataProcessing* inputData;

	void onAddFramePressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);
	void onSavestateHookPressed(wxCommandEvent& event);

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxBoxSizer* sizer, DataProcessing* input);
};