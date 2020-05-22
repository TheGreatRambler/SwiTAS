#pragma once

#include <wx/wx.h>

#include "../sharedNetworkCode/networkInterface.hpp"
#include "buttonConstants.hpp"
#include "buttonData.hpp"
#include "dataProcessing.hpp"

class TasRunner : public wxDialog {
private:
	std::shared_ptr<CommunicateWithNetwork> networkInstance;
	rapidjson::Document* mainSettings;
	DataProcessing* dataProcessing;

	wxBoxSizer* mainSizer;
	wxBoxSizer* hookSelectionSizer;

	wxSpinCtrl* firstSavestateHook;
	wxSpinCtrl* lastSavestateHook;

	wxBitmapButton* startTasHomebrew;
	wxBitmapButton* startTasArduino;
	// More will be added as needed

	wxBitmapButton* pauseTas;
	// Stopping will also close the dialog
	wxBitmapButton* stopTas;

	void onStartTasHomebrewPressed(wxCommandEvent& event);
	void onStartTasArduinoPressed(wxCommandEvent& event);

public:
	TasRunner(wxFrame* parent, std::shared_ptr<CommunicateWithNetwork> networkImp, rapidjson::Document* settings, DataProcessing* inputData);
};