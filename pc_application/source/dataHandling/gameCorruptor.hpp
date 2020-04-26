#pragma once

#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "dataProcessing.hpp"
#include "projectHandler.hpp"

class GameCorruptor : public wxDialog {
private:
	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	wxBoxSizer* mainSizer;

public:
	GameCorruptor(wxWindow* parent, std::shared_ptr<ProjectHandler> projHandler, std::shared_ptr<CommunicateWithNetwork> networkImp);
};