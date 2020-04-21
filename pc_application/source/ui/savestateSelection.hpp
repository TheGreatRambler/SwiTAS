#pragma once

#include <rapidjson/document.h>
#include <wx/event.h>
#include <wx/spinctrl.h>
#include <wx/utils.h>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "../helpers.hpp"
#include "drawingCanvas.hpp"

// This class handles both opening up a savestate and creating the first savestate
class SavestateSelection : public wxDialog {
private:
	wxBoxSizer* imageSizer;
	wxBoxSizer* buttonSizer;
	wxBoxSizer* fullSizer;

	// Sizers containing both the images and the dHash below
	wxBoxSizer* leftImageSizer;
	wxBoxSizer* rightImageSizer;

	wxStaticText* leftDHash;
	wxStaticText* rightDHash;

	wxStaticText* hammingDistance;

	rapidjson::Document* mainSettings;

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

public:
	SavestateSelection(rapidjson::Document* settings, bool isSavestateLoadDialog, std::shared_ptr<CommunicateWithNetwork> networkImp);

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