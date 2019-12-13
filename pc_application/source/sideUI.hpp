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
	DataProcessing* inputInstance;

public:
	SideUI () {
		// Some buttons for the left UI
		// https://developer.gnome.org/gtkmm-tutorial/stable/sec-multi-item-containers.html.en#buttonbox-example
		// https://www.lucidarme.me/gtkmm-example-2/
		playButtons.set_orientation (Gtk::ORIENTATION_HORIZONTAL);
		playButtons.set_layout (Gtk::BUTTONBOX_SPREAD);

		// Set images for buttons (instead of add_label)
		playPauseButton.set_image_from_icon_name ("pauseButton");
		frameAdvanceButton.set_image_from_icon_name ("frameAdvanceButton");

		playButtons.add (playPauseButton);
		playButtons.add (frameAdvanceButton);
	}

	void setInputInstance (DataProcessing* input) {
		inputInstance = input;
	}

	void addToGrid (Gtk::Grid* theGrid) {
		theGrid->attach (playButtons, 0, 0);
		// Use the scrolled window obtained from inputsInstance
		theGrid->attach (*inputInstance->getWindow (), 0, 1);
	}
}