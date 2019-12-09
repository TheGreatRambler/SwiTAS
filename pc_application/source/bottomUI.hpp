#pragma once

#include <cairomm/context.h>
#include <glibmm/main.h>
#include <gtkmm.h>
#include <map>
#include <utility>

#include "buttonData.hpp"
#include "dataProcessing.hpp"

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
std::map<Btn, Location>
    KeyLocs{
        {Btn::A, {10, 2}},    {Btn::B, {9, 3}},     {Btn::X, {9, 1}},
        {Btn::Y, {8, 2}},     {Btn::L, {3, 0}},     {Btn::R, {7, 0}},
        {Btn::ZL, {2, 0}},    {Btn::ZR, {8, 0}},    {Btn::DUP, {1, 1}},
        {Btn::DDOWN, {1, 3}}, {Btn::DLEFT, {0, 2}}, {Btn::DRIGHT, {2, 2}},
        {Btn::PLUS, {7, 1}},  {Btn::MINUS, {3, 1}}, {Btn::HOME, {6, 2}},
        {Btn::CAPT, {4, 2}},  {Btn::LS, {0, 0}},    {Btn::RS, {3, 3}},
    };

// Canvas that the joystick viewer is drawn to
// https://developer.gnome.org/gtkmm-tutorial/stable/sec-drawing-clock-example.html.en
class JoystickCanvas : public Gtk::DrawingArea {
public:
  JoystickCanvas() {
    // Don't know why this is needed
    // Force a redraw every 30 milliseconds
    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &JoystickCanvas::on_timeout), 30);
  }

protected:
  // Override default signal handler:
  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override {
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
      Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                       get_allocation().get_height());
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

  // Input instance to get inputs and such
  DataProcessing *inputInstance;

  // Grid containing the button viewer
  Gtk::Grid buttonViewer;

  // The images displayed (just here to keep a reference)
  std::map<Btn, Gtk::image *> onImages;
  std::map<Btn, Gtk::image *> offImages;

public:
  BottomUI() {
    // TODO set up joysticks
    // Add grid of buttons
    for (auto const &button : KeyLocs) {
      // Create on images and add them
      Gtk::Image *onImage =
          new Gtk::image(buttonMapping[button.first]->bottomInputViewIcon);
      onImages.insert(std::pair<Btn, Gtk::image *>(button.first, onImage));
      // Create off images and add them
      Gtk::Image *offImage =
          new Gtk::image(buttonMapping[button.first]->offIcon);
      offImages.insert(std::pair<Btn, Gtk::image *>(button.first, offImage));

      // Designate the off image as the default
      buttonViewer.attach(*offImage, button.second.x, button.second.y);
    }
  }

  void setInputInstance(DataProcessing *input) { inputInstance = input; }

  void setIconState(Btn button, bool state) {
    bool currentState = inputInstance->getButtonState(button);
    // Only update if there is a need to update
    if (currentState != state) {
      if (currentState) {
        buttonViewer.remove(*onImages[button]);
        buttonViewer.attach(*offImages[button], KeyLocs[button].x,
                            KeyLocs[button].y);
      } else {
        buttonViewer.remove(*offImages[button]);
        buttonViewer.attach(*onImages[button], KeyLocs[button].x,
                            KeyLocs[button].y);
      }

      // Set value in input instance
      inputInstance->setButtonState(button, state);
    }
  }

  void addToGrid(Gtk::Grid *theGrid) {
    theGrid->attach(leftJoystick, 0, 0);
    theGrid->attach(rightJoystick, 1, 0);
    theGrid->attach(buttonViewer, 2, 0);
  }

  ~BottomUI() {
    // Deallocate all the images
    for (auto const &image : onImages) {
      free(image.second);
    }
    for (auto const &image : offImages) {
      free(image.second);
    }
  }
}