#pragma once

#include <cfloat>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include <wx/checkbox.h>
#include <wx/dcbuffer.h>
#include <wx/display.h>
#include <wx/grid.h>
#include <wx/joystick.h>
#include <wx/listbox.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

#include "../data_handling/buttonData.hpp"
#include "../data_handling/dataProcessing.hpp"
#include "../data_handling/projectHandler.hpp"
#include "../helpers.hpp"
#include "drawingCanvas.hpp"

class ExtraInputMethods : public wxFrame {
private:
	DataProcessing* inputInstance;
	std::shared_ptr<ButtonData> buttonData;

	wxBoxSizer* mainSizer;

	uint8_t lastWasVisible = false;

	wxStaticBoxSizer* motionLeftSizer;
	wxStaticBoxSizer* accelLeftSizer;
	wxStaticBoxSizer* angularVelocityLeftSizer;
	wxStaticBoxSizer* angleLeftSizer;
	wxStaticBoxSizer* directionXLeftSizer;
	wxStaticBoxSizer* directionYLeftSizer;
	wxStaticBoxSizer* directionZLeftSizer;

	wxSpinCtrlDouble* accelXLeftCtrl;
	wxSpinCtrlDouble* accelYLeftCtrl;
	wxSpinCtrlDouble* accelZLeftCtrl;
	wxSpinCtrlDouble* angularVelocityXLeftCtrl;
	wxSpinCtrlDouble* angularVelocityYLeftCtrl;
	wxSpinCtrlDouble* angularVelocityZLeftCtrl;
	wxSpinCtrlDouble* angleXLeftCtrl;
	wxSpinCtrlDouble* angleYLeftCtrl;
	wxSpinCtrlDouble* angleZLeftCtrl;
	wxSpinCtrlDouble* directionXXLeftCtrl;
	wxSpinCtrlDouble* directionXYLeftCtrl;
	wxSpinCtrlDouble* directionXZLeftCtrl;
	wxSpinCtrlDouble* directionYXLeftCtrl;
	wxSpinCtrlDouble* directionYYLeftCtrl;
	wxSpinCtrlDouble* directionYZLeftCtrl;
	wxSpinCtrlDouble* directionZXLeftCtrl;
	wxSpinCtrlDouble* directionZYLeftCtrl;
	wxSpinCtrlDouble* directionZZLeftCtrl;

	wxStaticBoxSizer* motionRightSizer;
	wxStaticBoxSizer* accelRightSizer;
	wxStaticBoxSizer* angularVelocityRightSizer;
	wxStaticBoxSizer* angleRightSizer;
	wxStaticBoxSizer* directionXRightSizer;
	wxStaticBoxSizer* directionYRightSizer;
	wxStaticBoxSizer* directionZRightSizer;

	wxSpinCtrlDouble* accelXRightCtrl;
	wxSpinCtrlDouble* accelYRightCtrl;
	wxSpinCtrlDouble* accelZRightCtrl;
	wxSpinCtrlDouble* angularVelocityXRightCtrl;
	wxSpinCtrlDouble* angularVelocityYRightCtrl;
	wxSpinCtrlDouble* angularVelocityZRightCtrl;
	wxSpinCtrlDouble* angleXRightCtrl;
	wxSpinCtrlDouble* angleYRightCtrl;
	wxSpinCtrlDouble* angleZRightCtrl;
	wxSpinCtrlDouble* directionXXRightCtrl;
	wxSpinCtrlDouble* directionXYRightCtrl;
	wxSpinCtrlDouble* directionXZRightCtrl;
	wxSpinCtrlDouble* directionYXRightCtrl;
	wxSpinCtrlDouble* directionYYRightCtrl;
	wxSpinCtrlDouble* directionYZRightCtrl;
	wxSpinCtrlDouble* directionZXRightCtrl;
	wxSpinCtrlDouble* directionZYRightCtrl;
	wxSpinCtrlDouble* directionZZRightCtrl;

	wxStaticBoxSizer* firstTouchSizer;
	wxStaticBoxSizer* secondTouchSizer;
	wxBoxSizer* touchesSizer;
	wxStaticBoxSizer* mainTouchSizer;

	wxSpinCtrl* touchX1Ctrl;
	wxSpinCtrl* touchY1Ctrl;
	wxSpinCtrl* touchX2Ctrl;
	wxSpinCtrl* touchY2Ctrl;
	wxSpinCtrl* numberOfTouchesCtrl;

	wxStaticBoxSizer* mainMouseSizer;

	wxSpinCtrl* mouseXCtrl;
	wxSpinCtrl* mouseYCtrl;
	wxSpinCtrl* mouseVelocityXCtrl;
	wxSpinCtrl* mouseVelocityYCtrl;
	wxSpinCtrl* scrollVelocityXCtrl;
	wxSpinCtrl* scrollVelocityYCtrl;

	wxStaticBoxSizer* mainKeyboardKeysSizer;
	wxStaticBoxSizer* mainKeyboardModifiersSizer;
	wxStaticBoxSizer* mainMouseButtonsSizer;

	wxStaticBoxSizer* keyboardKeysSizer;
	wxStaticBoxSizer* keyboardModifiersSizer;
	wxStaticBoxSizer* mouseButtonsSizer;

	wxListCtrl* keyboardKeys;
	wxListCtrl* keyboardModifiers;
	wxListCtrl* mouseButtons;

	std::unordered_map<int, nn::hid::KeyboardKey> keyboardKeyIndices;
	std::unordered_map<int, nn::hid::KeyboardModifier> keyboardModifierIndices;
	std::unordered_map<int, nn::hid::MouseButton> mouseButtonIndices;

	std::unordered_map<nn::hid::KeyboardKey, int> keyboardKeyIndicesReverse;
	std::unordered_map<nn::hid::KeyboardModifier, int> keyboardModifierIndicesReverse;
	std::unordered_map<nn::hid::MouseButton, int> mouseButtonIndicesReverse;

	void motionValueChanged(wxSpinDoubleEvent& event);
	void touchValueChanged(wxSpinEvent& event);
	void numOfTouchesChanged(wxSpinEvent& event);
	void mouseValueChanged(wxSpinEvent& event);
	void keyboardKeysChanged(wxListEvent& event);
	void keyboardModifiersChanged(wxListEvent& event);
	void mouseButtonsChanged(wxListEvent& event);

	void onClose(wxCloseEvent& event);

public:
	ExtraInputMethods(wxFrame* parentFrame, DataProcessing* input, std::shared_ptr<ButtonData> data);

	void updateAllValues();
};

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

	wxBitmap* primary   = nullptr;
	wxBitmap* secondary = nullptr;

public:
	FrameViewerCanvas(wxFrame* parent, wxBitmap* defaultImage);

	void draw(wxDC& dc) override;

	// TODO have the ability to compare images
	void setPrimaryBitmap(wxBitmap* primaryBitmap);
	void setSecondaryBitmap(wxBitmap* secondaryBitmap);
};

class JoystickCanvas : public DrawingCanvas {
private:
	rapidjson::Document* mainSettings;

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

	wxBitmapButton* lockButton;

	void xValueSet(wxSpinEvent& event);
	void yValueSet(wxSpinEvent& event);

	void correctForCircleLock();

	void onMouseClick(wxMouseEvent& event);
	void onMouseDrag(wxMouseEvent& event);

public:
	JoystickCanvas(rapidjson::Document* settings, wxFrame* parent, DataProcessing* inputData, uint8_t leftJoy);

	void draw(wxDC& dc) override;

	void setXValue(int16_t x);
	void setYValue(int16_t y);

	wxBoxSizer* getSizer() {
		return widgetSizer;
	}

	wxBitmapButton* getLockButton() {
		return lockButton;
	}
};

class BottomUI {
private:
	rapidjson::Document* mainSettings;
	std::shared_ptr<ProjectHandler> projectHandler;

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

	// The editor for motion and touch data, not shown by default
	ExtraInputMethods* extraInputMethodsWindow;

	// The button mapping instance
	std::shared_ptr<ButtonData> buttonData;

	wxMenu* joystickSubMenu;
	uint8_t joysticksExist;

	wxJoystick* currentJoy;
	uint8_t currentJoyDefined;
	int lastButtonState;

	std::map<std::string, int> stringToButtonExtended {
		{ "LSX", 0 },
		{ "LSY", 1 },
		{ "RSX", 2 },
		{ "RSY", 3 },
	};

	// Custom accelerator IDs
	int screenshotExportID;

	// Menu popup
	wxMenu editMenu;

	// All of these kinda map to Btn values, there are just some extra Btn
	// Values after the normal ones
	std::unordered_map<int, int> joyButtonToSwitch;
	std::unordered_map<int, int> povToSwitch;
	std::unordered_map<int, int> axisButtonsToSwitch;

	// Will tell whether the axis should be flipped or not
	std::unordered_map<int, bool> axisDirection;

	std::unordered_map<int, int> leftStickAxis;
	std::unordered_map<int, int> rightStickAxis;

	// Put here so that buttons don't rapidly change state when held
	std::unordered_map<Btn, bool> lastState;
	int povLastState;

	// Menu item for joysticks, will be exclusively used
	// by the bottom UI, so that's why it is here
	void onJoystickMenuOpen(wxMenuEvent& event);

	wxString getJoyHexString(wxJoystick* joy);

	void onFrameViewerRightClick(wxContextMenuEvent& event);

	void exportImageView(wxCommandEvent& event);

public:
	BottomUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, DataProcessing* input, std::shared_ptr<ProjectHandler> projHandler);

	wxMenu* getJoystickMenu() {
		return joystickSubMenu;
	}

	FrameViewerCanvas* getFrameViewerCanvas() {
		return frameViewerCanvas;
	}

	// Just a random large number, apparently can't be larger than 76
	static constexpr int joystickSubmenuIDBase = 23;

	void recieveGameFramebuffer(std::vector<uint8_t> jpegBuffer);

	void refreshDataViews(uint8_t refreshFramebuffer);

	void toggleExtraInputMethodsWindow() {
		extraInputMethodsWindow->Show(!extraInputMethodsWindow->IsShown());
		refreshDataViews(true);
	}

	void listenToJoystick();

	void onJoystickSelect(wxCommandEvent& event);

	void onLeftJoystickLock(wxCommandEvent& event);
	void onRightJoystickLock(wxCommandEvent& event);
};