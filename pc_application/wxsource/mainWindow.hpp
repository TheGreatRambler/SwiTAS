#pragma once

#include <fstream>
#include <gdk/gdk.h>
#include <gtkmm/grid.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/menubar.h>
#include <gtkmm/window.h>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>

#include "bottomUI.hpp"
#include "buttonData.hpp"
#include "dataProcessing.hpp"
#include "sideUI.hpp"

class MainWindow : public wxFrame {
private:
	// Layout that contains all items, including the menubar
	Gtk::VBox mainLayout;
	// Layout that contains all the app content
	Gtk::HBox mainContent;
	// Layout that contains things on the right
	Gtk::VBox rightSideBox;
	// Grid containing the left hand side widgets
	Gtk::VBox leftGrid;
	// Grid containing the bottom widgets
	Gtk::HBox bottomGrid;
	// Menu bar containing "open file", etc...
	Gtk::MenuBar menuBar;

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
	bool on_key_press_event(GdkEventKey* event) override;

	void handlePreviousWindowTransform();

	void getGlobalSettings(rapidjson::Document* d);

public:
	MainWindow();

	void addGrids();

	void addMenuBar();

	void addLeftUI();

	void addBottomUI();

	~MainWindow();
};