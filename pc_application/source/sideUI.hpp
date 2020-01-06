#pragma once

#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/grid.h>

#include "dataProcessing.hpp"

class SideUI {
private:
	// Button group for play, pause, etc...
	Gtk::ButtonBox playButtons;
	// Buttons themselves
	Gtk::Button playPauseButton;
	Gtk::Button frameAdvanceButton;

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

public:
	SideUI();

	void setInputInstance(std::shared_ptr<DataProcessing> input);

	void addToGrid(Gtk::Grid* theGrid);
};