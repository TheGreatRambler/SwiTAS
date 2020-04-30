#include "gameCorruptor.hpp"

MemorySectionViewer::MemorySectionViewer(wxWindow* parent)
	: DrawingCanvas(parent, wxDefaultSize) {}

void MemorySectionViewer::draw(wxDC& dc) {
	// Will use https://github.com/WerWolv/EdiZon/blob/master/source/guis/gui_cheats.cpp#L340
}

GameCorruptor::GameCorruptor(wxWindow* parent, std::shared_ptr<ProjectHandler> projHandler, std::shared_ptr<CommunicateWithNetwork> networkImp)
	: wxDialog(parent, wxID_ANY, "Game Corruptor", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxMAXIMIZE) {
	projectHandler  = projHandler;
	networkInstance = networkImp;

	mainSizer = new wxBoxSizer(wxHORIZONTAL);

	memorySectionViewer = new MemorySectionViewer(this);

	// Implement RTC algorithms here
	// Starting with Vector Engine

	// TODO
	// Use drawing canvas to visualize memory regions
	// Add range selector aligned to the right byte value based on the engine
	// Have play and pause button and only allow memory editing while paused
	// Have this be entirely separate from the TASing, will need to figure that out
	// Probs have this as a dialog that replaces the main window and puts DataProcessing
	// into an unloaded savestate state

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

// clang-format off
BEGIN_EVENT_TABLE(GameCorruptor, wxDialog)
    EVT_IDLE(GameCorruptor::onIdle)
END_EVENT_TABLE()
// clang-format on

void GameCorruptor::onIdle(wxIdleEvent& event) {
	// Check networkInstance for memory info and enable everything
}