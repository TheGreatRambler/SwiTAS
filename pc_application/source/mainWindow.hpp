#pragma once

#include <fstream>
#include <gdk/gdk.h>
#include <gtkmm/grid.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/menubar.h>
#include <gtkmm/window.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>

#include "bottomUI.hpp"
#include "buttonData.hpp"
#include "dataProcessing.hpp"
#include "sideUI.hpp"

class MainWindow : public Gtk::Window {
private:
	// Layout that contains all items, including the menubar
	Gtk::VBox mainLayout;
	// Grid where all the layout stuff is dealt with
	Gtk::Grid mainGrid;
	// Grid containing the left hand side widgets
	Gtk::Grid leftGrid;
	// Grid containing the bottom widgets
	Gtk::Grid bottomGrid;
	// Menu bar containing "open file", etc...
	Gtk::MenuBar menuBar;

	// Main settings variable
	rapidjson::Document mainSettings;

	// Button data variable
	ButtonData buttonData;

	// The pointers to the classes containing the uis
	SideUI* sideUI;
	BottomUI* bottomUI;
	// Pointer to the class containing important input stuff
	DataProcessing* dataProcessingInstance;

	// Override default signal handler:
	bool on_key_press_event(GdkEventKey* event) override;

	void handlePreviousWindowTransform();

	std::string getFilePath(std::string path);

	void getGlobalSettings(rapidjson::Document* d);

public:
	MainWindow();

	void addGrids();

	void addMenuBar();

	void addLeftUI();

	void addBottomUI();

	~MainWindow();
};