#pragma once

//#include "../thirdParty/win32darkmode/DarkMode.h"
#include <ffms.h>
#include <fstream>
#include <memory>
#include <rapidjson/document.h>
#include <string>
#include <vector>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"

#include "../dataHandling/buttonData.hpp"
#include "../dataHandling/dataProcessing.hpp"
#include "../dataHandling/projectHandler.hpp"
#include "../helpers.hpp"
#include "bottomUI.hpp"
#include "debugWindow.hpp"
#include "sideUI.hpp"

class MainWindow : public wxFrame {
private:
	// Simplify things immesnly
	wxBoxSizer* mainSizer;

	// Main settings variable
	rapidjson::Document mainSettings;

	// Button data variable
	std::shared_ptr<ButtonData> buttonData;

	// Project handler, creates projects and saves them
	std::shared_ptr<ProjectHandler> projectHandler;

	// The pointers to the classes containing the uis
	std::shared_ptr<SideUI> sideUI;
	std::shared_ptr<BottomUI> bottomUI;
	// Pointer to the class containing important input stuff
	DataProcessing* dataProcessingInstance;
	// Networking stuff
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	// Main logging window
	wxLogWindow* logWindow;
	// Main debug command window
	DebugWindow* debugWindow;

	// Menubar
	wxMenuBar* menuBar;
	// Menubar menus
	wxWindowID selectIPID;
	wxWindowID setNameID;
	wxWindowID toggleLoggingID;
	wxWindowID toggleDebugMenuID;

	void handlePreviousWindowTransform();

	void handleMenuBar(wxCommandEvent& commandEvent);

	void onRecentVideosMenuOpen(wxMenuEvent& event) {
		// Pass on to project handler
		projectHandler->onRecentVideosMenuOpen(event);
	}

	void addMenuBar();
	void addStatusBar();

	// Override default signal handler:
	void keyDownHandler(wxKeyEvent& event);
	void OnSize(wxSizeEvent& event);

	bool askForIP();
	void handleNetworkQueues();

public:
	MainWindow();

	void endNetworking() {
		// Force end it
		networkInstance->endNetwork();
	}

	void onIdle(wxIdleEvent& event);

	// Called by wxApp
	void onStart();

	void onClose(wxCloseEvent& event);

	DECLARE_EVENT_TABLE();
};