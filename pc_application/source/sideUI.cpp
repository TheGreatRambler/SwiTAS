#include "sideUI.hpp"

SideUI::SideUI() {
	// Some buttons for the left UI
	// https://developer.gnome.org/gtkmm-tutorial/stable/sec-multi-item-containers.html.en#buttonbox-example
	// https://www.lucidarme.me/gtkmm-example-2/
	playButtons.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	playButtons.set_layout(Gtk::BUTTONBOX_SPREAD);

	// Set images for buttons (instead of add_label)
	playPauseButton.set_image_from_icon_name("pauseButton");
	frameAdvanceButton.set_image_from_icon_name("frameAdvanceButton");

	playButtons.add(playPauseButton);
	playButtons.add(frameAdvanceButton);
}

void SideUI::setInputInstance(std::shared_ptr<DataProcessing> input) {
	inputInstance = input;
}

void SideUI::addToGrid(Gtk::Grid* theGrid) {
	theGrid->attach(playButtons, 0, 0);
	// Use the scrolled window obtained from inputsInstance
	theGrid->attach(*inputInstance->getWindow(), 0, 1);
}