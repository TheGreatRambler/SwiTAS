#pragma once

#include <rapidjson/document.h>
#include <wx/event.h>
#include <wx/scrolwin.h>
#include <wx/spinctrl.h>
#include <wx/utils.h>
#include <wx/wrapsizer.h>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"
#include "../dataHandling/projectHandler.hpp"
#include "../helpers.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"
#include "drawingCanvas.hpp"

class SavestateLister : public wxDialog {
private:
	wxBoxSizer* mainSizer;

	wxWrapSizer* projectList;
	wxScrolledWindow* projectListHolder;
	DataProcessing* inputInstance;

	std::vector<DrawingCanvasBitmap*> savestateScreenshots;

	bool operationSuccessful = false;
	int selectedSavestate;

	void onSavestateHookSelect(wxMouseEvent& event);

public:
	SavestateLister(wxFrame* parent, DataProcessing* input);

	bool getOperationSuccessful() {
		return operationSuccessful;
	}

	int getSelectedSavestate() {
		return selectedSavestate;
	}
};

// This class handles both opening up a savestate and creating the first savestate
class SavestateSelection : public wxDialog {
private:
	const uint8_t NETWORK_CALLBACK_ID = 1;

	wxBoxSizer* imageSizer;
	wxBoxSizer* buttonSizer;
	wxBoxSizer* fullSizer;

	// Sizers containing both the images and the dHash below
	wxBoxSizer* leftImageSizer;
	wxBoxSizer* rightImageSizer;

	wxStaticText* leftDHash;
	wxStaticText* rightDHash;

	wxStaticText* hammingDistance;

	int dhashWidth;
	int dhashHeight;

	uint8_t paused   = true;
	uint8_t okCalled = false;

	rapidjson::Document* mainSettings;
	std::shared_ptr<ProjectHandler> projectHandler;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	// All the buttons
	wxBitmapButton* playButton;
	wxBitmapButton* pauseButton;
	wxBitmapButton* frameAdvanceButton;
	wxBitmapButton* autoFrameAdvanceButton;
	wxBitmapButton* okButton;

	// Canvases for showing the frames
	DrawingCanvasBitmap* currentFrame;
	DrawingCanvasBitmap* goalFrame;

	std::string leftDhashString;

	// Will be set if the dialog is supposed to load savestates, not create the first one
	bool savestateLoadDialog;

	bool operationSuccessful = false;
	bool autoFrameEnabled    = false;

	wxTimer* autoFrameAdvanceTimer;

	wxSpinCtrl* selectFrameAutomatically;
	wxSpinCtrl* autoIncrementDelay;

	// To view the frames, will use if needed
	DrawingCanvasBitmap* currentScreen;
	// Only use with savestate loading
	DrawingCanvasBitmap* savestateFrameTarget;

	void callOk();

	void registerFramebufferCallback();

	void onIdle(wxIdleEvent& event);
	void onAutoFrameAdvanceTimer(wxTimerEvent& event);

	void onPlay(wxCommandEvent& event);
	void onPause(wxCommandEvent& event);
	void onFrameAdvance(wxCommandEvent& event);
	void onAutoFrameAdvance(wxCommandEvent& event);
	void onOk(wxCommandEvent& event);

	void onClose(wxCloseEvent& event);
	void onResize(wxSizeEvent& event);

	void frameAdvance();

public:
	SavestateSelection(wxFrame* parent, rapidjson::Document* settings, std::shared_ptr<ProjectHandler> projHandler, bool isSavestateLoadDialog, std::shared_ptr<CommunicateWithNetwork> networkImp);

	// Will use ShowModel for this, act like a normal wxDialog

	bool getOperationSuccessful() {
		return operationSuccessful;
	}

	std::string getNewDhash() {
		return leftDhashString;
	}

	wxBitmap* getNewScreenshot() {
		return currentFrame->getBitmap();
	}

	void setTargetFrame(wxBitmap* targetBitmap, std::string targetDhash);

	DECLARE_EVENT_TABLE();
};