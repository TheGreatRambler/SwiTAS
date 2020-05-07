#pragma once

#include <rapidjson/document.h>
#include <wx/event.h>
#include <wx/scrolwin.h>
#include <wx/spinctrl.h>
#include <wx/utils.h>
#include <wx/wrapsizer.h>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "../dataHandling/dataProcessing.hpp"
#include "../dataHandling/projectHandler.hpp"
#include "../helpers.hpp"
#include "drawingCanvas.hpp"

// Class listing out the savestates in a grid so they can be selected
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
	SavestateLister(DataProcessing* input);

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

	rapidjson::Document* mainSettings;
	std::shared_ptr<ProjectHandler> projectHandler;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	// All the buttons
	wxBitmapButton* playButton;
	wxBitmapButton* pauseButton;
	wxBitmapButton* frameAdvanceButton;
	wxBitmapButton* okButton;

	// Canvases for showing the frames
	DrawingCanvasBitmap* currentFrame;
	DrawingCanvasBitmap* goalFrame;

	// Will be set if the dialog is supposed to load savestates, not create the first one
	bool savestateLoadDialog;

	bool operationSuccessful = false;

	wxSpinCtrl* selectFrameAutomatically;

	// To view the frames, will use if needed
	DrawingCanvasBitmap* currentScreen;
	// Only use with savestate loading
	DrawingCanvasBitmap* savestateFrameTarget;

	void callOk() {
		// Use this frame as the savestate
		operationSuccessful = true;
		Close(true);
	}

	void onIdle(wxIdleEvent& event);

	void onPlay(wxCommandEvent& event);
	void onPause(wxCommandEvent& event);
	void onFrameAdvance(wxCommandEvent& event);
	void onOk(wxCommandEvent& event);

	void onClose(wxCloseEvent& event);

public:
	SavestateSelection(rapidjson::Document* settings, std::shared_ptr<ProjectHandler> projHandler, bool isSavestateLoadDialog, std::shared_ptr<CommunicateWithNetwork> networkImp);

	// Will use ShowModel for this, act like a normal wxDialog

	bool getOperationSuccessful() {
		return operationSuccessful;
	}

	std::string getNewDhash() {
		return leftDHash->GetLabel().ToStdString();
	}

	wxBitmap* getNewScreenshot() {
		return currentFrame->getBitmap();
	}

	void setTargetFrame(wxBitmap* targetBitmap, std::string targetDhash);

	DECLARE_EVENT_TABLE();
};