#include "bottomUI.hpp"

ButtonGrid::ButtonGrid(wxFrame* parent, wxSize requiredSize, std::shared_ptr<ButtonData> data, DataProcessing* inputs)
	: DrawingCanvas(parent, requiredSize) {
	SetDoubleBuffered(false);

	buttonData             = data;
	totalCombinedImageSize = requiredSize;
	inputInstance          = inputs;

	setBackgroundColor(*wxWHITE);

	// Handle grid clicking
	Bind(wxEVT_LEFT_DOWN, &ButtonGrid::onGridClick, this);

	// Callback stuff
	inputInstance->setInputCallback(std::bind(&ButtonGrid::setIconState, this, std::placeholders::_1, std::placeholders::_2));

	for(auto const& button : buttonData->buttonMapping) {
		char buf[5];
		sprintf(buf, "%d-%d", button.second->gridX, button.second->gridY);
		locToButton[std::string(buf)] = button.first;
	}
}

void ButtonGrid::setIconState(Btn button, bool state) {
	// Just trigger a refresh (I'm too lazy)
	Refresh();
}

void ButtonGrid::draw(wxDC& dc) {
	int width;
	int height;
	GetSize(&width, &height);

	// Set scaling for the image to render without wxImage
	double scaleWidth  = (double)width / totalCombinedImageSize.GetWidth();
	double scaleHeight = (double)height / totalCombinedImageSize.GetHeight();
	dc.SetUserScale(scaleWidth, scaleHeight);

	// TODO not every image is a square :)
	for(auto const& button : buttonData->buttonMapping) {
		wxBitmap* bitmap;
		if(inputInstance->getButtonState(button.first)) {
			bitmap = button.second->resizedGridOnBitmap;
		} else {
			bitmap = button.second->resizedGridOffBitmap;
		}
		wxPoint location;
		location.x = button.second->gridX * bitmap->GetWidth();
		location.y = button.second->gridY * bitmap->GetHeight();

		dc.DrawBitmap(*bitmap, location, true);
	}
}

void ButtonGrid::onGridClick(wxMouseEvent& event) {
	wxPoint windowCoord = event.GetPosition();
	// Divide to handle scaling
	int width;
	int height;
	GetSize(&width, &height);

	// Want to remove everything after the decimal point
	int X = (int)(((float)windowCoord.x / width) * buttonData->KeyWidth);
	int Y = (int)(((float)windowCoord.y / height) * buttonData->KeyHeight);

	char buf[5];
	sprintf(buf, "%d-%d", X, Y);
	std::string key(buf);

	if(locToButton.count(key)) {
		// Element exists, time to trigger the click
		inputInstance->handleButtonInput(locToButton[key]);
		Refresh();
	}
}

FrameViewerCanvas::FrameViewerCanvas(wxFrame* parent, wxBitmap* defaultImage)
	: DrawingCanvas(parent, wxSize(1280, 720)) {
	// Needs to be able to fit the frames
	hasFrameToRender  = false;
	defaultBackground = defaultImage;
}

void FrameViewerCanvas::draw(wxDC& dc) {
	int width;
	int height;
	GetSize(&width, &height);
	if(!hasFrameToRender) {
		// Set scaling for the image to render without wxImage
		dc.SetUserScale((double)width / defaultBackground->GetWidth(), (double)height / defaultBackground->GetHeight());
		// Render the default image, that's it
		dc.DrawBitmap(*defaultBackground, 0, 0, false);
	} else {
		// Do thing
	}
}

JoystickCanvas::JoystickCanvas(wxFrame* parent, DataProcessing* inputData, uint8_t leftJoy)
	: DrawingCanvas(parent, wxSize(150, 150)) {
	// Should be a decent size
	isLeftJoystick = leftJoy;
	inputInstance  = inputData;

	// Create widgets
	xInput                     = new wxSpinCtrl(parent);
	yInput                     = new wxSpinCtrl(parent);
	canGoOutsideCircleCheckbox = new wxCheckBox(parent, wxID_ANY, wxEmptyString);

	xInput->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &JoystickCanvas::xValueSet, this);
	yInput->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &JoystickCanvas::yValueSet, this);

	xInput->SetMin(-30000);
	xInput->SetMax(30000);
	yInput->SetMin(-30000);
	yInput->SetMax(30000);

	inputSizer  = new wxBoxSizer(wxHORIZONTAL);
	widgetSizer = new wxBoxSizer(wxVERTICAL);

	inputSizer->Add(xInput, 0, wxEXPAND);
	inputSizer->Add(yInput, 0, wxEXPAND);
	inputSizer->Add(canGoOutsideCircleCheckbox, 0, wxEXPAND);

	widgetSizer->Add(this, 0, wxSHAPED | wxEXPAND);
	widgetSizer->Add(inputSizer, 1, wxEXPAND | wxALL);
}

void JoystickCanvas::draw(wxDC& dc) {
	// Do thing
	int width;
	int height;
	GetSize(&width, &height);

	wxPoint approximateMiddle((float)width / 2, (float)height / 2);

	dc.SetPen(*wxGREEN_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);

	dc.DrawCircle(approximateMiddle, approximateMiddle.x);

	int32_t joyX;
	int32_t joyY;

	if(isLeftJoystick) {
		joyX = inputInstance->getCurrentFrame()->LS_X;
		joyY = inputInstance->getCurrentFrame()->LS_Y;
	} else {
		joyX = inputInstance->getCurrentFrame()->RS_X;
		joyY = inputInstance->getCurrentFrame()->RS_Y;
	}

	int renderJoyX = (joyX / 30000.0f) * approximateMiddle.x + approximateMiddle.x;
	int renderJoyY = (joyY / 30000.0f) * approximateMiddle.y + approximateMiddle.y;

	int middleCircleRadius = std::floor((float)width / 20);

	// Optimise if the angle is nonexistent
	if(joyX != 0 && joyY != 0) {
		float joyAngle = std::atan2(joyY, joyX);

		// First, draw the rectangle
		dc.SetBrush(*wxMEDIUM_GREY_BRUSH);

		wxPoint joystickBarPoints[4];

		// Calculate the angle once and then mirror to use twice
		float barDrawingAngle = HELPERS::normalizeRadian(joyAngle + (M_PI / 2.0f));
		int offsetX           = middleCircleRadius * std::cos(barDrawingAngle);
		int offsetY           = middleCircleRadius * std::sin(barDrawingAngle);

		joystickBarPoints[0] = wxPoint(approximateMiddle.x + offsetX, approximateMiddle.y + offsetY);
		joystickBarPoints[1] = wxPoint(renderJoyX + offsetX, renderJoyY + offsetY);
		joystickBarPoints[2] = wxPoint(renderJoyX - offsetX, renderJoyY - offsetY);
		joystickBarPoints[3] = wxPoint(approximateMiddle.y - offsetX, approximateMiddle.y - offsetY);

		dc.DrawPolygon(4, joystickBarPoints, approximateMiddle.x, approximateMiddle.y);
	}

	// Draw the middle ball
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawCircle(approximateMiddle, middleCircleRadius);

	// Finally, draw the joystick itself
	dc.DrawCircle(renderJoyX, renderJoyY, middleCircleRadius + 5);
}

void JoystickCanvas::xValueSet(wxSpinEvent& event) {
	int position = event.GetPosition();
	if(position > 30000) {
		position = 30000;
		event.SetPosition(position);
	}

	if(position < -30000) {
		position = -30000;
		event.SetPosition(position);
	}

	if(isLeftJoystick) {
		inputInstance->getCurrentFrame()->LS_X = position;
	} else {
		inputInstance->getCurrentFrame()->RS_X = position;
	}
	Refresh();
}

void JoystickCanvas::yValueSet(wxSpinEvent& event) {
	int position = event.GetPosition();
	if(position > 30000) {
		position = 30000;
		event.SetPosition(position);
	}

	if(position < -30000) {
		position = -30000;
		event.SetPosition(position);
	}

	if(isLeftJoystick) {
		inputInstance->getCurrentFrame()->LS_Y = position;
	} else {
		inputInstance->getCurrentFrame()->RS_Y = position;
	}
	Refresh();
}

BottomUI::BottomUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, DataProcessing* input) {
	// TODO set up joysticks
	buttonData   = buttons;
	mainSettings = settings;

	inputInstance = input;

	// Game frame viewer

	mainSizer          = new wxBoxSizer(wxVERTICAL);
	horizontalBoxSizer = new wxBoxSizer(wxHORIZONTAL);

	leftJoystickDrawer = new JoystickCanvas(parentFrame, inputInstance, true);
	leftJoystickDrawer->setBackgroundColor(*wxWHITE);
	rightJoystickDrawer = new JoystickCanvas(parentFrame, inputInstance, false);
	rightJoystickDrawer->setBackgroundColor(*wxWHITE);

	wxSize gridSize;
	// Just to get a rough estimate
	int w = buttonData->KeyWidth * buttonData->buttonMapping[Btn::A]->resizedGridOffBitmap->GetWidth();
	int h = buttonData->KeyHeight * buttonData->buttonMapping[Btn::A]->resizedGridOffBitmap->GetHeight();
	gridSize.SetWidth(w);
	gridSize.SetHeight(h);

	buttonGrid = new ButtonGrid(parentFrame, gridSize, buttonData, inputInstance);

	frameViewerCanvas = new FrameViewerCanvas(parentFrame, new wxBitmap(HELPERS::resolvePath((*mainSettings)["videoViewerDefaultImage"].GetString()), wxBITMAP_TYPE_JPEG));

	// These take up much less space than the grid
	horizontalBoxSizer->Add(leftJoystickDrawer->getSizer(), 0, wxSHAPED | wxEXPAND);
	horizontalBoxSizer->Add(rightJoystickDrawer->getSizer(), 0, wxSHAPED | wxEXPAND);

	horizontalBoxSizer->Add(buttonGrid, 0, wxSHAPED | wxEXPAND);

	// Proportion HAS to be zero here, it's a requirment
	mainSizer->Add(frameViewerCanvas, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(horizontalBoxSizer, 1, wxEXPAND | wxALL);

	theGrid->Add(mainSizer, 3, wxEXPAND | wxALL);
}