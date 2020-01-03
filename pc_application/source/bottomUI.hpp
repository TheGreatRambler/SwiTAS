#pragma once

#include <cairomm/context.h>
#include <functional>
#include <gdkmm/rectangle.h>
#include <glibmm/main.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <map>
#include <utility>

#include "buttonData.hpp"
#include "dataProcessing.hpp"

// Location
struct Location {
	int x;
	int y;
};

// https://www.tablesgenerator.com/text_tables
//      0   1    2    3   4  5   6   7   8    9  10
//   +----+---+----+----+---+--+---+---+----+---+---+
// 0 | LS |   | ZL | L  |   |  |   | R | ZR |   |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 1 |    | ^ |    | -  |   |  |   | + |    | X |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 2 | <  |   | >  |    | C |  | H |   | Y  |   | A |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 3 |    | v |    | RS |   |  |   |   |    | B |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
std::map<Btn, Location> KeyLocs {
	{ Btn::A, { 10, 2 } },
	{ Btn::B, { 9, 3 } },
	{ Btn::X, { 9, 1 } },
	{ Btn::Y, { 8, 2 } },
	{ Btn::L, { 3, 0 } },
	{ Btn::R, { 7, 0 } },
	{ Btn::ZL, { 2, 0 } },
	{ Btn::ZR, { 8, 0 } },
	{ Btn::DUP, { 1, 1 } },
	{ Btn::DDOWN, { 1, 3 } },
	{ Btn::DLEFT, { 0, 2 } },
	{ Btn::DRIGHT, { 2, 2 } },
	{ Btn::PLUS, { 7, 1 } },
	{ Btn::MINUS, { 3, 1 } },
	{ Btn::HOME, { 6, 2 } },
	{ Btn::CAPT, { 4, 2 } },
	{ Btn::LS, { 0, 0 } },
	{ Btn::RS, { 3, 3 } },
};

// Canvas that the joystick viewer is drawn to
// https://developer.gnome.org/gtkmm-tutorial/stable/sec-drawing-clock-example.html.en
class JoystickCanvas : public Gtk::DrawingArea {
public:
	JoystickCanvas();

protected:
	// Override default signal handler:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

	bool on_timeout();
};

class BottomUI {
private:
	// Both joystick drawing areas
	JoystickCanvas leftJoystick;
	JoystickCanvas rightJoystick;

	// Input instance to get inputs and such
	DataProcessing* inputInstance;

	// Grid containing the button viewer
	Gtk::Grid buttonViewer;

	// The images displayed and their eventboxes (just here to keep a reference)
	std::map<Btn, std::pair<Gtk::Image*, Gtk::EventBox*>> images;

protected:
	bool onButtonPress(GdkEventButton* event, Btn button);

public:
	BottomUI();

	void setInputInstance(DataProcessing* input);

	void setIconState(Btn button, bool state);

	void addToGrid(Gtk::Grid* theGrid);

	~BottomUI();
};