#include "sideUI.hpp"

SideUI::SideUI() {
	// Some buttons for the left UI
	// https://developer.gnome.org/gtkmm-tutorial/stable/sec-multi-item-containers.html.en#buttonbox-example
	// https://www.lucidarme.me/gtkmm-example-2/
	playButtons.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	playButtons.set_layout(Gtk::BUTTONBOX_SPREAD);

	// Set images for buttons (instead of add_label)
	// Starts paused
	playPauseButton.set_image_from_icon_name("media-playback-start");
	frameAdvanceButton.set_image_from_icon_name("go-last");

	playButtons.add(playPauseButton);
	playButtons.add(frameAdvanceButton);

	playButtons.set_layout(Gtk::BUTTONBOX_EXPAND);

	// Add everything to the box
	everythingBox.pack_start(playButtons, Gtk::PACK_SHRINK);
}

void SideUI::setInputInstance(std::shared_ptr<DataProcessing> input) {
	inputInstance = input;
	everythingBox.pack_start(*inputInstance->getWindow(), Gtk::PACK_EXPAND_WIDGET);
}

void SideUI::addToGrid(Gtk::VBox* theGrid) {
	// Set minimum width to a quarter of the screen size
	// Doesn't work for some reason right now
	int widgetWidth = (int)(minimumSize * Gdk::Screen::get_default()->get_width());
	everythingBox.set_size_request(widgetWidth, -1);
	// Now a misnomer :)
	theGrid->pack_start(everythingBox);
}