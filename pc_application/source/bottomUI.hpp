#pragma once

#include <gtkmm.h>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <map>

#include "buttonData.hpp"

// Location
struct Location {
	int x;
	int y;
}

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
    {Btn::A, {10, 2}},
    {Btn::B, {9, 3}},
    {Btn::X, {9, 1}},
    {Btn::Y, {8, 2}},
    {Btn::L, {3, 0}},
    {Btn::R, {7, 0}},
    {Btn::ZL, {2, 0}},
    {Btn::ZR, {8, 0}},
    {Btn::DUP, {1, 1}},
    {Btn::DDOWN, {1, 3}},
    {Btn::DLEFT, {0, 2}},
    {Btn::DRIGHT, {2, 2}},
    {Btn::PLUS, {7, 1}},
    {Btn::MINUS, {3, 1}},
    {Btn::HOME, {6, 2}},
    {Btn::CAPT, {4, 2}},
    {Btn::LS, {0, 0}},
    {Btn::RS, {3, 3}}, 
};

// Canvas that the joystick viewer is drawn to
// https://developer.gnome.org/gtkmm-tutorial/stable/sec-drawing-clock-example.html.en
class JoystickCanvas : public Gtk::DrawingArea {
    public:
    JoystickCanvas() {
        // Don't know why this is needed
        // Force a redraw every 30 milliseconds
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &JoystickCanvas::on_timeout), 30);
    }

    protected:
    //Override default signal handler:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
        Gtk::Allocation allocation = get_allocation();
        const int width = allocation.get_width();
        const int height = allocation.get_height();

        // scale to unit square and translate (0, 0) to be (0.5, 0.5), i.e.
        // the center of the window
        cr->scale(width, height);
        cr->translate(0.5, 0.5);

        // TODO do the drawing
    }

    bool on_timeout() {
        // Apparently this is needed
        // force our program to redraw the entire clock.
        auto win = get_window();
        if (win) {
            Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
            // Invalidate the canvas so it is drawn again
            win->invalidate_rect(r, false);
        }
        return true;
    }
}

class BottomUI {
    private:
    // Both joystick drawing areas
    JoystickCanvas leftJoystick;
    JoystickCanvas rightJoystick;

    // Grid containing the button viewer
    Gtk::Grid buttonViewer;

    // The images displayed (just here to keep a reference)
    std::vector<Glib::RefPtr<Gtk::Image>> buttonImages;

    public:
    BottomUI() {
        // TODO set up joysticks
        // Add grid of buttons
        for (auto const& button : KeyLocs) {
            Gtk::Image* image = new Gtk::image(buttonMapping[button.first]->bottomInputViewIcon);
            buttonImages.push_back(image);
            buttonViewer.attach(*image, button.second.x, button.second.y);
        }
    }

    void addToGrid(Gtk::Grid* theGrid) {
        theGrid->attach(leftJoystick, 0, 0);
        theGrid->attach(rightJoystick, 1, 0);
        theGrid->attach(buttonViewer, 2, 0);
    }
}