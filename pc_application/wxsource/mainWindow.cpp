#include <memory>
#define RAPIDJSON_HAS_STDSTRING 1

#include "mainWindow.hpp"

// Override default signal handler:
void MainWindow::keyDownHandler(wxKeyEvent& event) {
	dataProcessingInstance->handleKeyboardInput(event.GetUnicodeKey());
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
	// Init PNGs
	wxImage::AddHandler(new wxPNGHandler());

	// Get the main settings
	getGlobalSettings(&mainSettings);

	wxIcon mainicon;
	mainicon.LoadFile(mainSettings["programIcon"].GetString());
	SetIcon(mainicon);

	mainSizer = std::make_shared<wxFlexGridSizer>(4, 4, 3, 3);

	// Set button data instance
	buttonData = std::make_shared<ButtonData>();
	// Load button data here
	buttonData->setupButtonMapping(&mainSettings);

	dataProcessingInstance = std::make_shared<DataProcessing>(buttonData);

	// UI instances
	sideUI   = std::make_shared<SideUI>(mainSettings, mainSizer.get(), dataProcessingInstance);
	bottomUI = std::make_shared<BottomUI>(buttonData, mainSizer.get(), dataProcessingInstance);

	// Add the top menubar
	addMenuBar();

	SetSizer(&mainSizer);
	SetMinSize(wxSize(270, 220));
	Center();

	// Override the keypress handler
	// add_events(Gdk::KEY_PRESS_MASK);
	handlePreviousWindowTransform();
}

void MainWindow::addMenuBar() {
	// https://www.lucidarme.me/gtkmm-example-13/
	// Add menubar to layout
	// mainLayout.pack_start(menuBar, Gtk::PACK_SHRINK);
}

MainWindow::~MainWindow() { }