#pragma once

#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "dataProcessing.hpp"
#include "projectHandler.hpp"
#include "../ui/drawingCanvas.hpp"

class GameCorruptor : public wxDialog {
private:
	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	wxBoxSizer* mainSizer;

	void onIdle(wxIdleEvent& event);

public:
	GameCorruptor(wxWindow* parent, std::shared_ptr<ProjectHandler> projHandler, std::shared_ptr<CommunicateWithNetwork> networkImp);

	DECLARE_EVENT_TABLE();
};