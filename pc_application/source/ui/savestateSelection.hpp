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

	// Sizers containing both the images and the dHash below
	wxBoxSizer* leftImageSizer;
	wxBoxSizer* rightImageSizer;

	wxStaticText* leftDHash;
	wxStaticText* rightDHash;

	wxStaticText* hammingDistance;

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

	bool operationSuccessful = false;

	// To view the frames, will use if needed
	DrawingCanvasBitmap* currentScreen;
	// Only use with savestate loading
	DrawingCanvasBitmap* savestateFrameTarget;

	void onIdle(wxIdleEvent& event);

	void onPlay(wxCommandEvent& event);
	void onPause(wxCommandEvent& event);
	void onFrameAdvance(wxCommandEvent& event);
	void onOk(wxCommandEvent& event);

	const uint16_t getHammingDistance(uint8_t* m1, uint8_t* m2, std::size_t size) {
		// https://gist.github.com/Miguellissimo/2faa7e3c3e1800a6bf97
		uint16_t counter = 0;

		for(std::size_t i = 0; i != size; ++i) {
			uint8_t diff = m1[i] ^ m2[i];

			diff = (diff & (uint8_t)0x55) + ((diff >> 1) & (uint8_t)0x55);
			diff = (diff & (uint8_t)0x33) + ((diff >> 2) & (uint8_t)0x33);
			diff = (diff & (uint8_t)0x0f) + ((diff >> 4) & (uint8_t)0x0f);

			counter += diff;
		}

		return counter;
	}

public:
	SavestateSelection(rapidjson::Document* settings, bool isSavestateLoadDialog);

	// Will use ShowModel for this, act like a normal wxDialog

	bool getOperationSuccessful() {
		return operationSuccessful;
	}

	DECLARE_EVENT_TABLE();
};