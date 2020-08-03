#pragma once

#include <wx/wx.h>

#include "../sharedNetworkCode/networkInterface.hpp"
#include "../ui/drawingCanvas.hpp"
#include "../ui/memoryViewer.hpp"
#include "dataProcessing.hpp"
#include "projectHandler.hpp"

class GameCorruptor : public wxDialog {
private:
	const uint8_t NETWORK_CALLBACK_ID = 2;

	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	wxBoxSizer* mainSizer;

	MemorySectionViewer* memorySectionViewer;

	void onIdle(wxIdleEvent& event);

public:
	GameCorruptor(wxWindow* parent, std::shared_ptr<ProjectHandler> projHandler, std::shared_ptr<CommunicateWithNetwork> networkImp);

	DECLARE_EVENT_TABLE();
};