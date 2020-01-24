#pragma once

#include <fstream>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <wx/wx.h>

#include "../dataHandling/buttonData.hpp"
#include "../dataHandling/dataProcessing.hpp"
#include "bottomUI.hpp"
#include "sideUI.hpp"

class MainWindow : public wxFrame {
private:
	// Layout that contains all items, including the menubar
	// Gtk::VBox mainLayout;
	// Layout that contains all the app content
	// Gtk::HBox mainContent;
	// Layout that contains things on the right
	// Gtk::VBox rightSideBox;
	// Grid containing the left hand side widgets
	// Gtk::VBox leftGrid;
	// Grid containing the bottom widgets
	// Gtk::HBox bottomGrid;
	// Menu bar containing "open file", etc...
	// Gtk::MenuBar menuBar;

	std::shared_ptr<wxFlexGridSizer> mainSizer;

	// Main settings variable
	rapidjson::Document mainSettings;

	// Button data variable
	std::shared_ptr<ButtonData> buttonData;

	// The pointers to the classes containing the uis
	std::shared_ptr<SideUI> sideUI;
	std::shared_ptr<BottomUI> bottomUI;
	// Pointer to the class containing important input stuff
	std::shared_ptr<DataProcessing> dataProcessingInstance;

	// Override default signal handler:
	void keyDownHandler(wxKeyEvent& event);

	void handlePreviousWindowTransform();

	void getGlobalSettings(rapidjson::Document* d);

	wxDECLARE_EVENT_TABLE();

public:
	MainWindow();

	void addMenuBar();

	~MainWindow();
};

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_CHAR_HOOK(MainWindow::keyDownHandler)
wxEND_EVENT_TABLE()
	// clang-format on