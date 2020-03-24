#pragma once

//#include "../thirdParty/win32darkmode/DarkMode.h"
#include <Windows.h>
#include <fstream>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <uxtheme.h>
#include <wx/wx.h>

#define CLIENT_IMP
#include "../../sharedNetworkCode/networkInterface.hpp"

#include "../dataHandling/buttonData.hpp"
#include "../dataHandling/dataProcessing.hpp"
#include "../dataHandling/projectHandler.hpp"
#include "../helpers.hpp"
#include "bottomUI.hpp"
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
	ProjectHandler* projectHandler;

	// The pointers to the classes containing the uis
	std::shared_ptr<SideUI> sideUI;
	std::shared_ptr<BottomUI> bottomUI;
	// Pointer to the class containing important input stuff
	DataProcessing* dataProcessingInstance;
	// Networking stuff
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	// Menubar
	wxMenuBar* menuBar;
	// Menubar menus
	wxWindowID selectIPID;
	wxWindowID setNameID;

	void handlePreviousWindowTransform();

	void getGlobalSettings(rapidjson::Document* d);

	void handleMenuBar(wxCommandEvent& commandEvent);

	void addMenuBar();
	void addStatusBar();

	// Override default signal handler:
	void keyDownHandler(wxKeyEvent& event);
	void OnSize(wxSizeEvent& event);

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

	//~MainWindow();

	DECLARE_EVENT_TABLE();
};