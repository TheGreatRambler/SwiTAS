#pragma once

#include <memory>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"

// Has a bunch of buttons for debugging purposes
class DebugWindow : public wxFrame {
private:
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	wxBoxSizer* buttonSizer;void onClose(wxCloseEvent& event);
	wxButton* pauseButton;
	wxButton* unpauseButton;

	void onPausePressed(wxCommandEvent& event);
	void onUnpausePressed(wxCommandEvent& event);

	

public:
	DebugWindow(std::shared_ptr<CommunicateWithNetwork> networkImp);

	DECLARE_EVENT_TABLE();
};