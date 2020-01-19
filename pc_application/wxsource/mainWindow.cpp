#define RAPIDJSON_HAS_STDSTRING 1

#include "mainWindow.hpp"

// Override default signal handler:
bool MainWindow::on_key_press_event(GdkEventKey* event) {
	// Handle keyboard input
	// A GDK int holding the pressed buttons
	guint key = event->keyval;
	// Send to the dataProcessing instance
	dataProcessingInstance->handleKeyboardInput(key);
	return Gtk::Window::on_key_press_event(event);
}

void MainWindow::handlePreviousWindowTransform() {
	// Resize and maximize as needed
	// TODO
}

void MainWindow::getGlobalSettings(rapidjson::Document* d) {
	std::ifstream settingsFile("../mainSettings.json");
	std::string content((std::istreambuf_iterator<char>(settingsFile)), (std::istreambuf_iterator<char>()));
	// Allow comments in JSON
	d->Parse<rapidjson::kParseCommentsFlag>(content.c_str());
}

MainWindow::MainWindow() {
	wxFrame((wxFrame*)NULL, -1, "NX TAS UI", wxDefaultPosition, wxSize(300, 200));
	// Get the main settings
	getGlobalSettings(&mainSettings);
	wxIcon mainicon;
	mainicon.LoadFile(mainSettings["programIcon"].GetString());
	SetIcon(mainicon);

	

	// Set icon to value in program settings
	set_icon_from_file(mainSettings["programIcon"].GetString());
	// Set button data instance
	buttonData = std::make_shared<ButtonData>();
	// Load button data here
	buttonData->setupButtonMapping(&mainSettings);
	// UI instances
	sideUI   = std::make_shared<SideUI>();
	bottomUI = std::make_shared<BottomUI>(buttonData);
	// Both UIs need this
	dataProcessingInstance = std::make_shared<DataProcessing>(buttonData);
	sideUI->setInputInstance(dataProcessingInstance);
	bottomUI->setInputInstance(dataProcessingInstance);
	// Add mainLayout to window
	add(mainLayout);
	// Add the top menubar
	addMenuBar();
	// Add left UI
	addLeftUI();
	// Add bottom UI
	addBottomUI();
	// Set button datas
	// Adding all the grids
	addGrids();
	// Show every item currently present
	show_all();
	// Override the keypress handler
	add_events(Gdk::KEY_PRESS_MASK);
	handlePreviousWindowTransform();
}

void MainWindow::addGrids() {
	mainContent.pack_start(leftGrid, Gtk::PACK_EXPAND_WIDGET);
	leftGrid.set_halign(Gtk::ALIGN_FILL);
	leftGrid.set_valign(Gtk::ALIGN_FILL);
	// Will have game viewer eventually
	rightSideBox.pack_start(bottomGrid, Gtk::PACK_SHRINK);
	rightSideBox.set_valign(Gtk::ALIGN_END);
	bottomGrid.set_halign(Gtk::ALIGN_FILL);
	mainContent.pack_start(rightSideBox);
	// Add all of them to the current layout
	mainLayout.pack_start(mainContent);
}

void MainWindow::addMenuBar() {
	// https://www.lucidarme.me/gtkmm-example-13/
	// Add menubar to layout
	mainLayout.pack_start(menuBar, Gtk::PACK_SHRINK);
}

void MainWindow::addLeftUI() {
	sideUI->addToGrid(&leftGrid);
}

void MainWindow::addBottomUI() {
	bottomUI->addToGrid(&bottomGrid);
}

MainWindow::~MainWindow() {}