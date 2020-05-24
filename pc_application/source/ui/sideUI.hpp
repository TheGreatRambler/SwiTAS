#pragma once

#include <cstdint>
#include <memory>
#include <rapidjson/document.h>
#include <wx/dcbuffer.h>
#include <wx/notebook.h>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"
#include "../helpers.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"
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
	const uint8_t NETWORK_CALLBACK_ID = 3;

	rapidjson::Document* mainSettings;
	std::shared_ptr<CommunicateWithNetwork> networkInterface;
	std::shared_ptr<ProjectHandler> projectHandler;
	wxFrame* parent;

	wxBoxSizer* verticalBoxSizer;

	uint8_t tethered                = false;
	uint8_t controllerEventRecieved = false;
	uint8_t autoRunActive           = false;

	wxBitmapButton* addFrameButton;
	wxBitmapButton* frameAdvanceButton;
	wxBitmapButton* savestateHookCreateButton;
	wxBitmapButton* savestateHookLoadButton;
	wxBitmapButton* savestateHookModifyButton;
	wxBitmapButton* playerAddButton;
	wxBitmapButton* playerRemoveButton;

	wxComboBox* playerSelect;

	wxBoxSizer* buttonSizer;

	// Sizer holding the inputs and the fancy viewer next to them
	wxBoxSizer* inputsViewSizer;

	FrameCanvas* frameDrawer;

	wxBoxSizer* autoFrameSizer;
	wxBitmapButton* autoFrameStart;
	wxBitmapButton* autoFrameEnd;
	wxSpinCtrl* autoRunFramesPerSecond;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	DataProcessing* inputData;

	void setPlayerInfo(uint8_t size, uint8_t selected, bool force);
	void playerSelected(wxCommandEvent& event);

	void autoRunIntervalChanged(wxSpinEvent& event);
	void sendAutoRunData();

	void onAddFramePressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);
	void onSavestateHookCreatePressed(wxCommandEvent& event);
	void onSavestateHookLoadPressed(wxCommandEvent& event);
	void onSavestateHookModifyPressed(wxCommandEvent& event);
	void onPlayerAddPressed(wxCommandEvent& event);
	void onPlayerRemovePressed(wxCommandEvent& event);
	void onStartAutoFramePressed(wxCommandEvent& event);
	void onEndAutoFramePressed(wxCommandEvent& event);

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ProjectHandler> projHandler, wxBoxSizer* sizer, DataProcessing* input, std::shared_ptr<CommunicateWithNetwork> networkImp);

	void onIdle(wxIdleEvent& event);

	bool createSavestateHook();
	bool loadSavestateHook(int block);

	void recieveAutoRunData(std::shared_ptr<ControllerData> controllerData);
	void handleUnexpectedControllerSize();

	void untether();
	void tether();
};