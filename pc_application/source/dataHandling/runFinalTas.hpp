#pragma once

#define BUFSIZE 5120

#include <wx/process.h>
#include <wx/spinctrl.h>
#include <wx/wfstream.h>
#include <wx/wx.h>

#include "../helpers.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"
#include "../sharedNetworkCode/serializeUnserializeData.hpp"
#include "buttonConstants.hpp"
#include "buttonData.hpp"
#include "dataProcessing.hpp"

// This port is defined in code, may change
// Will probably define in settings.json
#define SWITCH_FTP_PORT 5000

class TasRunner : public wxDialog {
private:
	enum RUNNING_COMMAND : uint8_t {
		NO_COMMAND,
		UPLOAD_SCRIPT,
	};

	std::shared_ptr<CommunicateWithNetwork> networkInstance;
	rapidjson::Document* mainSettings;
	DataProcessing* dataProcessing;

	SerializeProtocol serializeProtocol;

	wxBoxSizer* mainSizer;
	wxBoxSizer* hookSelectionSizer;

	wxSpinCtrl* firstSavestateHook;
	wxSpinCtrl* lastSavestateHook;

	wxTextCtrl* consoleLog;

	wxBitmapButton* startTasHomebrew;
	wxBitmapButton* startTasArduino;
	// More will be added as needed

	// wxBitmapButton* pauseTas;
	// Stopping will also close the dialog
	wxBitmapButton* stopTas;

	uint8_t currentWorkingPlayer;
	wxProcess* commandProcess;
	RUNNING_COMMAND currentRunningCommand = RUNNING_COMMAND::NO_COMMAND;
	std::vector<wxString> playerFiles;
	std::vector<std::string> scriptPaths;
	wxString ftpPath;

	void onCommandDone(wxProcessEvent& event);
	void uploadScript();

	void onIdle(wxIdleEvent& event);

	void onStartTasHomebrewPressed(wxCommandEvent& event);
	void onStartTasArduinoPressed(wxCommandEvent& event);

	void onStopTasPressed(wxCommandEvent& event);

public:
	TasRunner(wxFrame* parent, std::shared_ptr<CommunicateWithNetwork> networkImp, rapidjson::Document* settings, DataProcessing* inputData);

	DECLARE_EVENT_TABLE();
};