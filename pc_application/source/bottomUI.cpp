#include "bottomUI.hpp"

JoystickCanvas::JoystickCanvas() {
	// Don't know why this is needed
	// Force a redraw every 30 milliseconds
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &JoystickCanvas::on_timeout), 30);
}

// Override default signal handler:
bool JoystickCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	Gtk::Allocation allocation = get_allocation();
	const int width            = allocation.get_width();
	const int height           = allocation.get_height();

	// scale to unit square and translate (0, 0) to be (0.5, 0.5), i.e.
	// the center of the window
	cr->scale(width, height);
	cr->translate(0.5, 0.5);

	// TODO do the drawing

	// Sure
	return true;
}

bool JoystickCanvas::on_timeout() {
	// Apparently this is needed
	// force our program to redraw the entire clock.
	auto win = get_window();
	if(win) {
		Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
		// Invalidate the canvas so it is drawn again
		win->invalidate_rect(r, false);
	}
	return true;
}

bool BottomUI::onButtonPress(GdkEventButton* event, Btn button) {
	// This button has just been clicked, notify the dataProcess
	inputInstance->toggleButtonState(button);
	return true;
}

BottomUI::BottomUI(std::shared_ptr<ButtonData> buttons) {
	// TODO set up joysticks
	buttonData = buttons;
	// Add grid of buttons
	for(auto const& button : KeyLocs) {
		// Add the images (the pixbuf can and will be changed later)
		std::shared_ptr<Gtk::Image> image = std::make_shared<Gtk::Image>(buttonData->buttonMapping[button.first].offIcon);
		// Add the eventbox
		std::shared_ptr<Gtk::EventBox> eventBox = std::make_shared<Gtk::EventBox>();
		eventBox->add(*image);
		eventBox->set_events(Gdk::BUTTON_PRESS_MASK);
		eventBox->signal_button_press_event().connect(sigc::bind<Btn>(sigc::mem_fun(*this, &BottomUI::onButtonPress), button.first));

		images.insert(std::pair<Btn, std::pair<std::shared_ptr<Gtk::Image>, std::shared_ptr<Gtk::EventBox>>>(button.first, { image, eventBox }));

		// Designate the off image as the default
		buttonViewer.attach(*eventBox, button.second.x, button.second.y);
	}
}

void BottomUI::setInputInstance(std::shared_ptr<DataProcessing> input) {
	inputInstance = input;
	inputInstance->setInputCallback(std::bind(&BottomUI::setIconState, this, std::placeholders::_1, std::placeholders::_2));
}

void BottomUI::setIconState(Btn button, bool state) {
	if(state) {
		// Set the image to the on image
		images[button].first->set(buttonData->buttonMapping[button].onIcon);
	} else {
		// Set the image to the off image
		images[button].first->set(buttonData->buttonMapping[button].offIcon);
	}

	// Don't set value in input instance because it
	// Was the one that sent us here
}

void BottomUI::addToGrid(Gtk::HBox* theGrid) {
	theGrid->pack_start(leftJoystick);
	theGrid->pack_start(rightJoystick);
	theGrid->pack_start(buttonViewer);
}

BottomUI::~BottomUI() {
	// Deallocate all the images
	// for(auto const& image : images) {
	// Free images and eventbox
	// free(image.second.first);
	// free(image.second.second);
	//}
}