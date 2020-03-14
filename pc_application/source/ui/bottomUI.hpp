#pragma once

#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <unistd.h>
#include <utility>
#include <wx/checkbox.h>
#include <wx/dcbuffer.h>
#include <wx/grid.h>
#include <wx/joystick.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

#include "../dataHandling/buttonData.hpp"
#include "../dataHandling/dataProcessing.hpp"
#include "../helpers.hpp"
#include "drawingCanvas.hpp"

class ButtonGrid : public DrawingCanvas {
private:
	// The button mapping instance
	std::shared_ptr<ButtonData> buttonData;

	// Size of all images together
	wxSize totalCombinedImageSize;

	// Input instance to get inputs and such
	DataProcessing* inputInstance;

	// Hashmap for location
	std::unordered_map<std::string, Btn> locToButton;

public:
	ButtonGrid(wxFrame* parent, wxSize requiredSize, std::shared_ptr<ButtonData> data, DataProcessing* inputs);

	void draw(wxDC& dc) override;

	void onGridClick(wxMouseEvent& event);
};

class FrameViewerCanvas : public DrawingCanvas {
private:
	wxBitmap* defaultBackground;
	uint8_t hasFrameToRender;

public:
	FrameViewerCanvas(wxFrame* parent, wxBitmap* defaultImage);

	void draw(wxDC& dc) override;
};

class JoystickCanvas : public DrawingCanvas {
private:
	// Input instance to get inputs and such
	DataProcessing* inputInstance;

	// Used to keep the two apart
	uint8_t isLeftJoystick;

	// Handle manual number setting
	wxSpinCtrl* xInput;
	wxSpinCtrl* yInput;

	// Checkbox for if you can drag cursor outside of circle
	wxCheckBox* canGoOutsideCircleCheckbox;

	wxBoxSizer* inputSizer;
	wxBoxSizer* widgetSizer;

	void xValueSet(wxSpinEvent& event);
	void yValueSet(wxSpinEvent& event);

	void correctForCircleLock();

	void onMouseClick(wxMouseEvent& event);
	void onMouseDrag(wxMouseEvent& event);

public:
	JoystickCanvas(wxFrame* parent, DataProcessing* inputData, uint8_t leftJoy);

	void draw(wxDC& dc) override;

	wxBoxSizer* getSizer() {
		return widgetSizer;
	}
};

class BottomUI {
private:
	rapidjson::Document* mainSettings;

	// Input instance to get inputs and such
	DataProcessing* inputInstance;

	wxBoxSizer* mainSizer;

	wxBoxSizer* horizontalBoxSizer;

	// Needed for joysticks
	wxFrame* parent;

	JoystickCanvas* leftJoystickDrawer;
	JoystickCanvas* rightJoystickDrawer;

	// The true button grid
	ButtonGrid* buttonGrid;

	// The fancy viewer for the live framedata from the switch
	FrameViewerCanvas* frameViewerCanvas;

	// The button mapping instance
	std::shared_ptr<ButtonData> buttonData;

	wxJoystick* currentJoy;
	uint8_t currentJoyDefined;

	std::map<std::string, int> stringToButtonExtended {
		{ "LSX", 0 },
		{ "LSY", 1 },
		{ "RSX", 2 },
		{ "RSY", 3 },
	};

	// All of these kinda map to Btn values, there are just some extra Btn
	// Values after the normal ones
	std::unordered_map<int, int> joyButtonToSwitch;
	std::unordered_map<int, int> povToSwitch;
	std::unordered_map<int, int> axisToSwitch;

	// Menu item for joysticks, will be exclusively used
	// by the bottom UI, so that's why it is here
	wxMenu* joystickSubMenu;
	void onJoystickMenuOpen(wxMenuEvent& event);

	wxString getJoyHexString(wxJoystick* joy);

	void refreshDataViews();

public:
	BottomUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, DataProcessing* input);

	wxMenu* getJoystickSubmenu() {
		return joystickSubMenu;
	}

	// Just a random large number, apparently can't be larger than 76
	static constexpr int joystickSubmenuIDBase = 23;

	void onJoystickChange(wxJoystickEvent& event);
	void onJoystickSelect(wxCommandEvent& event);
};