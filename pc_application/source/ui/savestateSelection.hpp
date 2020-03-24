#pragma once

#include <rapidjson/document.h>
#include <wx/event.h>
#include <wx/utils.h>
#include <wx/wx.h>

#include "../helpers.hpp"
#include "drawingCanvas.hpp"

// This class handles both opening up a savestate and creating the first savestate
class SavestateSelection : public wxDialog {
private:
	wxBoxSizer* imageSizer;
	wxBoxSizer* buttonSizer;
	wxBoxSizer* fullSizer;

	rapidjson::Document* mainSettings;

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

	// To view the frames, will use if needed
	DrawingCanvasBitmap* currentScreen;
	// Only use with savestate loading
	DrawingCanvasBitmap* savestateFrameTarget;

	void onIdle(wxIdleEvent& event);

	void onPlay(wxCommandEvent& event);
	void onPause(wxCommandEvent& event);
	void onFrameAdvance(wxCommandEvent& event);
	void onOk(wxCommandEvent& event);

public:
	SavestateSelection(rapidjson::Document* settings, bool isSavestateLoadDialog);

	// Will use ShowModel for this, act like a normal wxDialog

	DECLARE_EVENT_TABLE();
};