#include "debugWindow.hpp"

DebugWindow::DebugWindow(std::shared_ptr<CommunicateWithNetwork> networkImp)
	: wxFrame(NULL, wxID_ANY, "Debug Menu", wxDefaultPosition, wxDefaultSize) {
	// Start hidden
	Hide();
	networkInstance = networkImp;

	buttonSizer   = new wxBoxSizer(wxVERTICAL);
	pauseButton   = new wxButton(this, wxID_ANY, "Pause");
	unpauseButton = new wxButton(this, wxID_ANY, "Unpause");

	pauseButton->Bind(wxEVT_BUTTON, &DebugWindow::onPausePressed, this);
	unpauseButton->Bind(wxEVT_BUTTON, &DebugWindow::onUnpausePressed, this);

	buttonSizer->Add(pauseButton, 1);
	buttonSizer->Add(unpauseButton, 1);

	SetSizer(buttonSizer);
	buttonSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

// clang-format off
BEGIN_EVENT_TABLE(DebugWindow, wxFrame)
	EVT_CLOSE(DebugWindow::onClose)
END_EVENT_TABLE()
// clang-format on

void DebugWindow::onClose(wxCloseEvent& event) {
	// Only hide, not delete
	Show(false);
}

void DebugWindow::onPausePressed(wxCommandEvent& event) {
	// clang-format off
	ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::PAUSE_DEBUG;
	})
	// clang-format on
}
void DebugWindow::onUnpausePressed(wxCommandEvent& event) {
	// clang-format off
	ADD_TO_QUEUE(SendFlag, networkInstance, {
		data.actFlag = SendInfo::UNPAUSE_DEBUG;
	})
	// clang-format on
}