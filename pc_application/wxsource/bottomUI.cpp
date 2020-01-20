#include "bottomUI.hpp"

JoystickCanvas::JoystickCanvas(wxFrame* parent) {
	// Initialize base class
	wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"));
}

// Override default signal handler:
void JoystickCanvas::draw() {
	// Use nanovg to draw a circle
	// SetCurrent sets the GL context
	// Now can use nanovg
	SetCurrent();
	/*
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLint)200, (GLint)200);
	glColor3f(1.0, c_, c_);

	glBegin(GL_POLYGON);
	glVertex3f(-0.5, -0.5, 5 * cos(rotate_));
	glVertex3f(-0.5, 0.5, 5 * cos(rotate_));
	glVertex3f(0.5, 0.5, -5 * cos(rotate_));
	glVertex3f(0.5, -0.5, -5 * cos(rotate_));
	glEnd();
	*/
	// Render
	SwapBuffers();
}

void JoystickCanvas::OnIdle(wxIdleEvent& event) {
	// Draw
	draw();
	// Dunno what this does
	event.RequestMore();
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

void BottomUI::addToGrid(wxFlexGridSizer* theGrid) {
	theGrid->Add();
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