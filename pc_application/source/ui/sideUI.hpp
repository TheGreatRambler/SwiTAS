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
	std::shared_ptr<ProjectHandler> projectHandler;

	wxBoxSizer* verticalBoxSizer;

	bool tethered = false;

	wxBitmapButton* addFrameButton;
	wxBitmapButton* frameAdvanceButton;
	wxBitmapButton* savestateHookCreateButton;
	wxBitmapButton* savestateHookLoadButton;
	wxBitmapButton* playerAddButton;
	wxBitmapButton* playerRemoveButton;

	wxComboBox* playerSelect;

	wxBoxSizer* buttonSizer;

	// Sizer holding the inputs and the fancy viewer next to them
	wxBoxSizer* inputsViewSizer;

	FrameCanvas* frameDrawer;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	DataProcessing* inputData;

	void setPlayerInfo(uint8_t size, uint8_t selected);
	void playerSelected(wxCommandEvent& event);

	void onAddFramePressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);
	void onSavestateHookCreatePressed(wxCommandEvent& event);
	void onSavestateHookLoadPressed(wxCommandEvent& event);
	void onPlayerAddPressed(wxCommandEvent& event);
	void onPlayerRemovePressed(wxCommandEvent& event);

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ProjectHandler> projHandler, wxBoxSizer* sizer, DataProcessing* input, std::shared_ptr<CommunicateWithNetwork> networkImp);

	bool createSavestateHook();
	bool loadSavestateHook(int block);

	void untether() {
		// Will need more indication
		// TODO have switch itself notify the PC when fishy buisness is going on
		// So it can untether itself
		// wxLogMessage("Untether Switch");
		frameAdvanceButton->Enable(false);
		inputData->setTethered(false);
		tethered = false;
	}
	void tether() {
		// wxLogMessage("Tether Switch");
		frameAdvanceButton->Enable(true);
		inputData->setTethered(true);
		tethered = true;
	}
};