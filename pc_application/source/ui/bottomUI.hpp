#pragma once

#include <functional>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <unistd.h>
#include <utility>
#include <wx/dcbuffer.h>
#include <wx/grid.h>
#include <wx/wx.h>

#include "../dataHandling/buttonData.hpp"
#include "../dataHandling/dataProcessing.hpp"
#include "../helpers.hpp"
#include "drawingCanvas.hpp"

class ButtonGrid : public DrawingCanvas {
private:
	// Location
	struct Location {
		int x;
		int y;
	};

	// The button mapping instance
	std::shared_ptr<ButtonData> buttonData;

	// Size of all images together
	wxSize totalCombinedImageSize;

	// Input instance to get inputs and such
	DataProcessing* inputInstance;

	// Handle changing icon state
	void setIconState(Btn button, bool state);

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
public:
	JoystickCanvas(wxFrame* parent);

	void draw(wxDC& dc) override;
};

class BottomUI {
private:
	rapidjson::Document* mainSettings;

	// Input instance to get inputs and such
	DataProcessing* inputInstance;

	wxBoxSizer* mainSizer;

	wxBoxSizer* horizontalBoxSizer;

	JoystickCanvas* leftJoystickDrawer;
	JoystickCanvas* rightJoystickDrawer;

	// The true button grid
	ButtonGrid* buttonGrid;

	// The fancy viewer for the live framedata from the switch
	FrameViewerCanvas* frameViewerCanvas;

	// The button mapping instance
	std::shared_ptr<ButtonData> buttonData;

public:
	BottomUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, DataProcessing* input);
};