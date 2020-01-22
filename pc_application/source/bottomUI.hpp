#pragma once

#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <wx/glcanvas.h>
#include <wx/wx.h>
#ifdef __WXMAC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <unistd.h>

#include "buttonData.hpp"
#include "dataProcessing.hpp"

// Canvas that the joystick viewer is drawn to
// https://developer.gnome.org/gtkmm-tutorial/stable/sec-drawing-clock-example.html.en

// https://wiki.wxwidgets.org/WxGLCanvas
/*
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

	// Both joystick drawing areas
	JoystickCanvas leftJoystick;
	JoystickCanvas rightJoystick;

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

	// Grid containing the button viewer
	wxGridSizer buttonGrid;

	// The button mapping instance
	std::shared_ptr<ButtonData> buttonData;

	// The images displayed and their eventboxes (just here to keep a reference)
	std::map<Btn, std::pair<std::shared_ptr<Gtk::Image>, std::shared_ptr<Gtk::EventBox>>> images;

protected:
	bool onButtonPress(GdkEventButton* event, Btn button);

public:
	BottomUI(std::shared_ptr<ButtonData> buttons);

	void setInputInstance(std::shared_ptr<DataProcessing> input);

	void setIconState(Btn button, bool state);

	void addToGrid(wxFlexGridSizer* theGrid);

	~BottomUI();
};
*/

class JoystickCanvas : public wxGLCanvas {
private:
	wxGLContext* co;
	bool init;

public:
	// https://wiki.wxwidgets.org/WxGLCanvas#Multiple_Canvases
	JoystickCanvas(wxFrame* parent);

	void draw();

	void OnIdle(wxIdleEvent& event);
	void OnResize(wxIdleEvent& event);

	void SetupViewport();
};

class renderImageInGrid : public wxGridCellRenderer {
private:
	std::shared_ptr<wxBitmap> theBitmap;

public:
	renderImageInGrid(std::shared_ptr<wxBitmap> bitmap);

	virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);

	void setBitmap(std::shared_ptr<wxBitmap> bitmap);
};

class BottomUI {
private:
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

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

	std::shared_ptr<wxBoxSizer> horizontalBoxSizer;

	std::shared_ptr<JoystickCanvas> leftJoystick;
	std::shared_ptr<JoystickCanvas> rightJoystick;

	std::shared_ptr<wxFrame> leftJoystickFrame;
	std::shared_ptr<wxFrame> rightJoystickFrame;

	// Grid containing the button viewer
	std::shared_ptr<wxGrid> buttonGrid;

	// The button mapping instance
	std::shared_ptr<ButtonData> buttonData;

protected:
	bool onButtonPress(GdkEventButton* event, Btn button);

public:
	BottomUI(std::shared_ptr<ButtonData> buttons, wxFlexGridSizer* theGrid, std::shared_ptr<DataProcessing> input);

	void setIconState(Btn button, bool state);

	~BottomUI();
};

// clang-format off
BEGIN_EVENT_TABLE(JoystickCanvas, wxGLCanvas)
	EVT_IDLE(JoystickCanvas::OnIdle)
	EVT_SIZE(JoystickCanvas::OnResize)
END_EVENT_TABLE()
// clang-format on