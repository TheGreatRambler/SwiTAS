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

MainWindow::MainWindow() {
	// UI instances
	sideUI   = new SideUI();
	bottomUI = new BottomUI();
	// Both UIs need this
	dataProcessingInstance = new DataProcessing();
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
	// Adding all the grids
	addGrids();
	// Override the keypress handler
	add_events(Gdk::KEY_PRESS_MASK);
	// Get the main settings
	Helpers::getGlobalSettings(&mainSettings);
	handlePreviousWindowTransform();
}

void MainWindow::addGrids() {
	// Add left grid spanning the whole left side
	mainGrid.attach(leftGrid, 0, 0, 1, 2);
	// Add bottom grid spanning the rest of the bottom
	mainGrid.attach(bottomGrid, 1, 1, 1, 1);
	// Shows all the items in this container
	mainGrid.show_all();
	// Add all of them to the current layout
	mainLayout.pack_start(mainGrid, Gtk::PACK_SHRINK);
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

MainWindow::~MainWindow() {
	delete sideUI;
	delete bottomUI;
	delete dataProcessingInstance;
}