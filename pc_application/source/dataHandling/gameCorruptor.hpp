#pragma once

#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "../ui/drawingCanvas.hpp"
#include "dataProcessing.hpp"
#include "projectHandler.hpp"

class MemorySectionViewer : public DrawingCanvas {
private:
public:
	MemorySectionViewer(wxFrame* parent);

	virtual void draw(wxDC& dc) override;
};

class GameCorruptor : public wxDialog {
private:
	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	wxBoxSizer* mainSizer;

	MemorySectionViewer* memorySectionViewer;

	void onIdle(wxIdleEvent& event);

public:
	GameCorruptor(wxWindow* parent, std::shared_ptr<ProjectHandler> projHandler, std::shared_ptr<CommunicateWithNetwork> networkImp);

	DECLARE_EVENT_TABLE();
};