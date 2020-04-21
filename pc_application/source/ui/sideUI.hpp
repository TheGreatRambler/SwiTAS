#pragma once

#include <cstdint>
#include <memory>
#include <rapidjson/document.h>
#include <wx/dcbuffer.h>
#include <wx/notebook.h>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "../dataHandling/dataProcessing.hpp"
#include "../helpers.hpp"
#include "drawingCanvas.hpp"
#include "savestateSelection.hpp"

class FrameCanvas : public DrawingCanvas {
private:
	DataProcessing* inputData;

	FrameNum currentFirst;
	FrameNum currentLast;

	FrameNum currentFrame;
	FrameNum currentRunFrame;
	FrameNum currentImageFrame;

	wxRect firstRect;

	void rangeUpdated(FrameNum first, FrameNum last);
	void currentFrameUpdated(FrameNum frame, FrameNum runFrame, FrameNum imageFrame);

public:
	FrameCanvas(wxFrame* parent, DataProcessing* dataProcessing);

	virtual void draw(wxDC& dc) override;
};

class SideUI {
private:
	rapidjson::Document* mainSettings;
	std::shared_ptr<CommunicateWithNetwork> networkInterface;

	wxBoxSizer* verticalBoxSizer;

	wxBitmapButton* addFrameButton;
	wxBitmapButton* frameAdvanceButton;
	wxBitmapButton* savestateHookButton;

	wxNotebook* inputsNotebook;

	wxBoxSizer* buttonSizer;

	// Sizer holding the inputs and the fancy viewer next to them
	wxBoxSizer* inputsViewSizer;

	FrameCanvas* frameDrawer;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	DataProcessing* inputData;

	bool shouldIgnorePageChange = false;

	void onAddFramePressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);
	void onSavestateHookPressed(wxCommandEvent& event);

	void OnNotebookClose(wxCloseEvent& event);
	void inputsPageChanged(wxBookCtrlEvent& event);

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxBoxSizer* sizer, DataProcessing* input, std::shared_ptr<CommunicateWithNetwork> networkImp);

	bool createSavestateHook();
	bool loadSavestateHook(SavestateBlockNum block);
};