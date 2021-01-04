#pragma once

#include <cstdint>
#include <memory>
#include <rapidjson/document.h>
#include <wx/dcbuffer.h>
#include <wx/notebook.h>
#include <wx/wx.h>

#include "../data_handling/dataProcessing.hpp"
#include "../helpers.hpp"
#include "../shared_network_code/networkInterface.hpp"
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
	void currentFrameUpdated(
		FrameNum frame, FrameNum runFrame, FrameNum imageFrame);

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
	wxBitmapButton* branchAddButton;
	wxBitmapButton* branchRemoveButton;

	wxComboBox* playerSelect;
	wxComboBox* branchSelect;
	wxSpinCtrl* runFinalTasStartingDelay;

	wxBoxSizer* buttonSizer;

	// Sizer holding the inputs and the fancy viewer next to them
	wxBoxSizer* inputsViewSizer;

	FrameCanvas* frameDrawer;

	std::function<void()> incrementFrameCallback;

	wxBoxSizer* autoFrameSizer;
	wxBitmapButton* autoFrameStart;
	wxBitmapButton* autoFrameEnd;
	wxSpinCtrl* autoRunFramesPerSecond;

	wxString typeChoices[(uint8_t)TasValueToRecord::NUM_OF_TYPES];

	wxCheckBox* autoRunWithFramebuffer;
	wxChoice* valueToRecord;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	DataProcessing* inputData;

	void setPlayerInfo(uint8_t size, uint8_t selected, bool force);
	void playerSelected(wxCommandEvent& event);
	void setBranchInfo(uint8_t size, uint8_t selected, bool force);
	void branchSelected(wxCommandEvent& event);

	void finalTasFrameDelayChanged(wxSpinEvent& event);

	void onAddFramePressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);
	void onSavestateHookCreatePressed(wxCommandEvent& event);
	void onSavestateHookLoadPressed(wxCommandEvent& event);
	void onSavestateHookModifyPressed(wxCommandEvent& event);
	void onPlayerAddPressed(wxCommandEvent& event);
	void onPlayerRemovePressed(wxCommandEvent& event);
	void onBranchAddPressed(wxCommandEvent& event);
	void onBranchRemovePressed(wxCommandEvent& event);
	void onStartAutoFramePressed(wxCommandEvent& event);
	void onEndAutoFramePressed(wxCommandEvent& event);

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings,
		std::shared_ptr<ProjectHandler> projHandler, wxBoxSizer* sizer,
		DataProcessing* input,
		std::shared_ptr<CommunicateWithNetwork> networkImp,
		std::function<void()> runFrameCallback);

	void onIdle(wxIdleEvent& event);

	bool createSavestateHook();
	bool loadSavestateHook(int block);

	void handleUnexpectedControllerSize();

	uint8_t getAutoRunActive() {
		return autoRunActive;
	}

	int getAutoRunDelay() {
		return autoRunFramesPerSecond->GetValue();
	}

	void enableAdvance() {
		frameAdvanceButton->Enable(true);
	}

	void disableAdvance() {
		frameAdvanceButton->Enable(false);
	}

	void sendAutoRunData();

	void untether();
	void tether();
};