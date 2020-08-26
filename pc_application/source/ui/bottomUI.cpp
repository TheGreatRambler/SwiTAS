#include "bottomUI.hpp"

ExtraInputMethods::ExtraInputMethods(wxFrame* parentFrame, DataProcessing* input)
	: wxFrame(parentFrame, wxID_ANY, "Motion and Touch Editor", wxDefaultPosition, wxSize(800, 400), wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT) {
	inputInstance = input;

	// Start hidden
	Hide();

	mainSizer = new wxBoxSizer(wxHORIZONTAL);

	motionLeftSizer          = new wxBoxSizer(wxVERTICAL);
	accelLeftSizer           = new wxBoxSizer(wxHORIZONTAL);
	angularVelocityLeftSizer = new wxBoxSizer(wxHORIZONTAL);
	angleLeftSizer           = new wxBoxSizer(wxHORIZONTAL);

	// clang-format off
	accelXLeftCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0);
	accelYLeftCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0);
	accelZLeftCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0);
	angularVelocityXLeftCtrl = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0);
	angularVelocityYLeftCtrl = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0);
	angularVelocityZLeftCtrl = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0);
	angleXLeftCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0);
	angleYLeftCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0);
	angleZLeftCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0);
	// clang-format on

	// Only those values representable as a float are counted
	accelXLeftCtrl->SetDigits(FLT_DIG);
	accelYLeftCtrl->SetDigits(FLT_DIG);
	accelZLeftCtrl->SetDigits(FLT_DIG);
	angularVelocityXLeftCtrl->SetDigits(FLT_DIG);
	angularVelocityYLeftCtrl->SetDigits(FLT_DIG);
	angularVelocityZLeftCtrl->SetDigits(FLT_DIG);
	angleXLeftCtrl->SetDigits(FLT_DIG);
	angleYLeftCtrl->SetDigits(FLT_DIG);
	angleZLeftCtrl->SetDigits(FLT_DIG);

	accelXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	accelYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	accelZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angularVelocityXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angularVelocityYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angularVelocityZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angleXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angleYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angleZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);

	accelXLeftCtrl->SetToolTip("Set acceleration X value for left joycon");
	accelYLeftCtrl->SetToolTip("Set acceleration Y value for left joycon");
	accelZLeftCtrl->SetToolTip("Set acceleration Z value for left joycon");
	angularVelocityXLeftCtrl->SetToolTip("Set angular velocity X value for left joycon");
	angularVelocityYLeftCtrl->SetToolTip("Set angular velocity Y value for left joycon");
	angularVelocityZLeftCtrl->SetToolTip("Set angular velocity Z value for left joycon");
	angleXLeftCtrl->SetToolTip("Set angle X value for left joycon");
	angleYLeftCtrl->SetToolTip("Set angle Y value for left joycon");
	angleZLeftCtrl->SetToolTip("Set angle Z value for left joycon");

	accelLeftSizer->Add(accelXLeftCtrl, 0, wxEXPAND);
	accelLeftSizer->Add(accelYLeftCtrl, 0, wxEXPAND);
	accelLeftSizer->Add(accelZLeftCtrl, 0, wxEXPAND);
	angularVelocityLeftSizer->Add(angularVelocityXLeftCtrl, 0, wxEXPAND);
	angularVelocityLeftSizer->Add(angularVelocityYLeftCtrl, 0, wxEXPAND);
	angularVelocityLeftSizer->Add(angularVelocityZLeftCtrl, 0, wxEXPAND);
	angleLeftSizer->Add(angleXLeftCtrl, 0, wxEXPAND);
	angleLeftSizer->Add(angleYLeftCtrl, 0, wxEXPAND);
	angleLeftSizer->Add(angleZLeftCtrl, 0, wxEXPAND);

	motionLeftSizer->Add(accelLeftSizer, 1, wxEXPAND);
	motionLeftSizer->Add(angularVelocityLeftSizer, 1, wxEXPAND);
	motionLeftSizer->Add(angleLeftSizer, 1, wxEXPAND);

	motionRightSizer          = new wxBoxSizer(wxVERTICAL);
	accelRightSizer           = new wxBoxSizer(wxHORIZONTAL);
	angularVelocityRightSizer = new wxBoxSizer(wxHORIZONTAL);
	angleRightSizer           = new wxBoxSizer(wxHORIZONTAL);

	// clang-format off
	accelXRightCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0);
	accelYRightCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0);
	accelZRightCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0);
	angularVelocityXRightCtrl = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0);
	angularVelocityYRightCtrl = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0);
	angularVelocityZRightCtrl = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0);
	angleXRightCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0);
	angleYRightCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0);
	angleZRightCtrl           = new wxSpinCtrlDouble(this,
		wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0);
	// clang-format on

	// Only those values representable as a float are counted
	accelXRightCtrl->SetDigits(FLT_DIG);
	accelYRightCtrl->SetDigits(FLT_DIG);
	accelZRightCtrl->SetDigits(FLT_DIG);
	angularVelocityXRightCtrl->SetDigits(FLT_DIG);
	angularVelocityYRightCtrl->SetDigits(FLT_DIG);
	angularVelocityZRightCtrl->SetDigits(FLT_DIG);
	angleXRightCtrl->SetDigits(FLT_DIG);
	angleYRightCtrl->SetDigits(FLT_DIG);
	angleZRightCtrl->SetDigits(FLT_DIG);

	accelXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	accelYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	accelZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angularVelocityXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angularVelocityYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angularVelocityZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angleXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angleYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);
	angleZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &MotionAndTouchWidget::motionValueChanged, this);

	accelXRightCtrl->SetToolTip("Set acceleration X value for right joycon");
	accelYRightCtrl->SetToolTip("Set acceleration Y value for right joycon");
	accelZRightCtrl->SetToolTip("Set acceleration Z value for right joycon");
	angularVelocityXRightCtrl->SetToolTip("Set angular velocity X value for right joycon");
	angularVelocityYRightCtrl->SetToolTip("Set angular velocity Y value for right joycon");
	angularVelocityZRightCtrl->SetToolTip("Set angular velocity Z value for right joycon");
	angleXRightCtrl->SetToolTip("Set angle X value for right joycon");
	angleYRightCtrl->SetToolTip("Set angle Y value for right joycon");
	angleZRightCtrl->SetToolTip("Set angle Z value for right joycon");

	accelRightSizer->Add(accelXRightCtrl, 0, wxEXPAND);
	accelRightSizer->Add(accelYRightCtrl, 0, wxEXPAND);
	accelRightSizer->Add(accelZRightCtrl, 0, wxEXPAND);
	angularVelocityRightSizer->Add(angularVelocityXRightCtrl, 0, wxEXPAND);
	angularVelocityRightSizer->Add(angularVelocityYRightCtrl, 0, wxEXPAND);
	angularVelocityRightSizer->Add(angularVelocityZRightCtrl, 0, wxEXPAND);
	angleRightSizer->Add(angleXRightCtrl, 0, wxEXPAND);
	angleRightSizer->Add(angleYRightCtrl, 0, wxEXPAND);
	angleRightSizer->Add(angleZRightCtrl, 0, wxEXPAND);

	motionRightSizer->Add(accelRightSizer, 1, wxEXPAND);
	motionRightSizer->Add(angularVelocityRightSizer, 1, wxEXPAND);
	motionRightSizer->Add(angleRightSizer, 1, wxEXPAND);

	mainSizer->Add(motionLeftSizer, 0);
	mainSizer->Add(motionRightSizer, 0);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void ExtraInputMethods::motionValueChanged(wxSpinDoubleEvent& event) {
	wxSpinCtrlDouble* widget = (wxSpinCtrlDouble*)event.GetEventObject();
	if(widget == accelXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ACCEL_X_LEFT, (float)widget->GetValue());
	} else if(widget == accelYLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ACCEL_Y_LEFT, (float)widget->GetValue());
	} else if(widget == accelZLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ACCEL_Z_LEFT, (float)widget->GetValue());
	} else if(widget == angularVelocityXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::GYRO_X_LEFT, (float)widget->GetValue());
	} else if(widget == angularVelocityYLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::GYRO_Y_LEFT, (float)widget->GetValue());
	} else if(widget == angularVelocityZLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::GYRO_Z_LEFT, (float)widget->GetValue());
	} else if(widget == angleXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ANGLE_X_LEFT, (float)widget->GetValue());
	} else if(widget == angleXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ANGLE_Y_LEFT, (float)widget->GetValue());
	} else if(widget == angleXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ANGLE_Z_LEFT, (float)widget->GetValue());
	}

	if(widget == accelXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ACCEL_X_RIGHT, (float)widget->GetValue());
	} else if(widget == accelYRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ACCEL_Y_RIGHT, (float)widget->GetValue());
	} else if(widget == accelZRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ACCEL_Z_RIGHT, (float)widget->GetValue());
	} else if(widget == angularVelocityXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::GYRO_X_RIGHT, (float)widget->GetValue());
	} else if(widget == angularVelocityYRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::GYRO_Y_RIGHT, (float)widget->GetValue());
	} else if(widget == angularVelocityZRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::GYRO_Z_RIGHT, (float)widget->GetValue());
	} else if(widget == angleXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ANGLE_X_RIGHT, (float)widget->GetValue());
	} else if(widget == angleXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ANGLE_Y_RIGHT, (float)widget->GetValue());
	} else if(widget == angleXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(ControllerNumberValues::ANGLE_Z_RIGHT, (float)widget->GetValue());
	}
}

void ExtraInputMethods::updateAllValues() {
	accelXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ACCEL_X_LEFT));
	accelYLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ACCEL_Y_LEFT));
	accelZLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ACCEL_Z_LEFT));
	angularVelocityXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::GYRO_X_LEFT));
	angularVelocityYLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::GYRO_Y_LEFT));
	angularVelocityZLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::GYRO_Z_LEFT));
	angleXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ANGLE_X_LEFT));
	angleXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ANGLE_Y_LEFT));
	angleXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ANGLE_Z_LEFT));

	accelXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ACCEL_X_RIGHT));
	accelYRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ACCEL_Y_RIGHT));
	accelZRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ACCEL_Z_RIGHT));
	angularVelocityXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::GYRO_X_RIGHT));
	angularVelocityYRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::GYRO_Y_RIGHT));
	angularVelocityZRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::GYRO_Z_RIGHT));
	angleXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ANGLE_X_RIGHT));
	angleXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ANGLE_Y_RIGHT));
	angleXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(ControllerNumberValues::ANGLE_Z_RIGHT));
}

ButtonGrid::ButtonGrid(wxFrame* parent, wxSize requiredSize, std::shared_ptr<ButtonData> data, DataProcessing* inputs)
	: DrawingCanvas(parent, requiredSize) {
	SetDoubleBuffered(false);

	buttonData             = data;
	totalCombinedImageSize = requiredSize;
	inputInstance          = inputs;

	setBackgroundColor(HELPERS::getDefaultWindowBackground());

	// Handle grid clicking
	Bind(wxEVT_LEFT_DOWN, &ButtonGrid::onGridClick, this);

	for(auto const& button : buttonData->buttonMapping) {
		char buf[5];
		sprintf(buf, "%u-%u", button.second->gridX, button.second->gridY);
		locToButton[std::string(buf)] = button.first;
	}
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
		if(inputInstance->getButtonCurrent(button.first)) {
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

	std::string key = wxString::Format("%u-%u", X, Y).ToStdString();
	if(locToButton.count(key)) {
		// Element exists, time to trigger the click
		inputInstance->triggerButton(locToButton[key]);
		Refresh();
	}
}

FrameViewerCanvas::FrameViewerCanvas(wxFrame* parent, wxBitmap* defaultImage)
	: DrawingCanvas(parent, wxSize(1280, 720)) {
	// Needs to be able to fit the frames
	defaultBackground = defaultImage;
}

void FrameViewerCanvas::draw(wxDC& dc) {
	int width;
	int height;
	GetSize(&width, &height);
	// Set scaling for the image to render without wxImage
	dc.SetUserScale((double)width / defaultBackground->GetWidth(), (double)height / defaultBackground->GetHeight());
	if(primary && !secondary) {
		dc.DrawBitmap(*primary, 0, 0, false);
	} else if(primary && secondary) {
		// No action for now
		// Will have fancy blending
		dc.DrawBitmap(*defaultBackground, 0, 0, false);
	} else {
		// Render the default image, that's it
		dc.DrawBitmap(*defaultBackground, 0, 0, false);
	}
}

void FrameViewerCanvas::setPrimaryBitmap(wxBitmap* primaryBitmap) {
	if(primary) {
		delete primary;
	}

	primary = primaryBitmap;
	Refresh();
}

void FrameViewerCanvas::setSecondaryBitmap(wxBitmap* secondaryBitmap) {
	if(secondary) {
		delete secondary;
	}

	secondary = secondaryBitmap;
}

JoystickCanvas::JoystickCanvas(rapidjson::Document* settings, wxFrame* parent, DataProcessing* inputData, uint8_t leftJoy)
	: DrawingCanvas(parent, wxSize(150, 150)) {
	// Should be a decent size
	isLeftJoystick = leftJoy;
	inputInstance  = inputData;
	mainSettings   = settings;

	setBackgroundColor(HELPERS::getDefaultWindowBackground());

	// Create widgets
	xInput                     = new wxSpinCtrl(parent, wxID_ANY, "x", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	yInput                     = new wxSpinCtrl(parent, wxID_ANY, "y", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	canGoOutsideCircleCheckbox = new wxCheckBox(parent, wxID_ANY, "Clamp");

	if(leftJoy) {
		xInput->SetToolTip("Edit X value for left joystick");
		yInput->SetToolTip("Edit Y value for left joystick");
	} else {
		xInput->SetToolTip("Edit X value for right joystick");
		yInput->SetToolTip("Edit Y value for right joystick");
	}

	canGoOutsideCircleCheckbox->SetToolTip("Lock joystick value to within circle");

	lockButton = HELPERS::getBitmapButton(parent, mainSettings, "joystickLockButton");

	xInput->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &JoystickCanvas::xValueSet, this);
	yInput->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &JoystickCanvas::yValueSet, this);

	Bind(wxEVT_LEFT_DOWN, &JoystickCanvas::onMouseClick, this);
	Bind(wxEVT_MOTION, &JoystickCanvas::onMouseDrag, this);

	xInput->SetRange(ButtonData::axisMin, ButtonData::axisMax);
	yInput->SetRange(ButtonData::axisMin, ButtonData::axisMax);

	inputSizer  = new wxBoxSizer(wxHORIZONTAL);
	widgetSizer = new wxBoxSizer(wxVERTICAL);
	inputSizer->Add(yInput, 0, wxEXPAND);
	inputSizer->Add(canGoOutsideCircleCheckbox, 0, wxEXPAND);

	widgetSizer->Add(this, 0, wxSHAPED | wxALIGN_CENTER_HORIZONTAL);
	widgetSizer->Add(xInput, 0, wxEXPAND);
	widgetSizer->Add(inputSizer, 0, wxEXPAND | wxALL);
	widgetSizer->Add(lockButton, 0, wxEXPAND | wxALL);
}

void JoystickCanvas::draw(wxDC& dc) {
	// Do thing
	int width;
	int height;
	GetSize(&width, &height);

	wxPoint approximateMiddle((float)width / 2, (float)height / 2);

	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxWHITE_BRUSH);

	dc.DrawCircle(approximateMiddle, approximateMiddle.x);

	int16_t joyX;
	int16_t joyY;

	if(isLeftJoystick) {
		joyX = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_X);
		joyY = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_Y);
	} else {
		joyX = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_X);
		joyY = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_Y);
	}

	// Flip the height to resemble the coordinate system
	joyY *= -1;

	int renderJoyX = ((float)joyX / ButtonData::axisMax) * approximateMiddle.x + approximateMiddle.x;
	int renderJoyY = ((float)joyY / ButtonData::axisMax) * approximateMiddle.y + approximateMiddle.y;

	int middleCircleRadius = std::floor((float)width / 20);

	// Optimise if the angle is nonexistent
	if(joyX != 0 || joyY != 0) {
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

		dc.DrawPolygon(4, joystickBarPoints);
	}

	// Draw the middle ball
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawCircle(approximateMiddle, middleCircleRadius);

	// Finally, draw the joystick itself
	dc.DrawCircle(renderJoyX, renderJoyY, middleCircleRadius + 5);

	// Show the actual value underneath
	if(isLeftJoystick) {
		xInput->SetValue(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_X));
		yInput->SetValue(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_Y));
	} else {
		xInput->SetValue(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_X));
		yInput->SetValue(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_Y));
	}
}

void JoystickCanvas::correctForCircleLock() {
	if(canGoOutsideCircleCheckbox->IsChecked()) {
		int x;
		int y;
		if(isLeftJoystick) {
			x = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_X);
			y = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_Y);
		} else {
			x = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_X);
			y = inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_Y);
		}

		// This corrects for circle lock if the checkbox is set
		// https://math.stackexchange.com/a/127615
		int radiusSquared = std::pow(x, 2) + std::pow(y, 2);
		if(radiusSquared > std::pow(ButtonData::axisMax, 2)) {
			// Have to clamp it
			x = ButtonData::axisMax * (x / std::sqrt(radiusSquared));
			y = ButtonData::axisMax * (y / std::sqrt(radiusSquared));
		}

		if(isLeftJoystick) {
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::LEFT_X, x);
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::LEFT_Y, y);
		} else {
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::RIGHT_X, x);
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::RIGHT_Y, y);
		}
	}
}

void JoystickCanvas::onMouseClick(wxMouseEvent& event) {
	wxPoint loc = event.GetPosition();
	if(GetScreenRect().Contains(ClientToScreen(loc))) {
		int width;
		int height;
		GetSize(&width, &height);

		int16_t scaledX = ((float)loc.x / width) * ButtonData::axisMax * 2 - ButtonData::axisMax;
		// Y is flipped
		int16_t scaledY = (((float)loc.y / height) * ButtonData::axisMax * 2 - ButtonData::axisMax) * -1;

		// Mutiply by twice the radius and then subtract the radius to get the middle
		if(isLeftJoystick) {
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::LEFT_X, scaledX);
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::LEFT_Y, scaledY);
		} else {
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::RIGHT_X, scaledX);
			inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::RIGHT_Y, scaledY);
		}

		correctForCircleLock();

		event.Skip();

		Refresh();
	}
}

void JoystickCanvas::onMouseDrag(wxMouseEvent& event) {
	if(event.Dragging()) {
		// Pass it on
		onMouseClick(event);
	}
}

void JoystickCanvas::xValueSet(wxSpinEvent& event) {
	int position = event.GetPosition();
	if(position > ButtonData::axisMax) {
		position = ButtonData::axisMax;
	}

	if(position < ButtonData::axisMin) {
		position = ButtonData::axisMin;
	}

	setXValue(position);

	if(isLeftJoystick) {
		event.SetPosition(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_X));
	} else {
		event.SetPosition(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_X));
	}
}

void JoystickCanvas::yValueSet(wxSpinEvent& event) {
	int position = event.GetPosition();
	if(position > ButtonData::axisMax) {
		position = ButtonData::axisMax;
	}

	if(position < ButtonData::axisMin) {
		position = ButtonData::axisMin;
	}

	setYValue(position);

	if(isLeftJoystick) {
		event.SetPosition(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::LEFT_Y));
	} else {
		event.SetPosition(inputInstance->getNumberValueCurrentJoystick(ControllerNumberValues::RIGHT_Y));
	}
}

void JoystickCanvas::setXValue(int16_t x) {
	if(isLeftJoystick) {
		inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::LEFT_X, x);
		correctForCircleLock();
	} else {
		inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::RIGHT_X, x);
		correctForCircleLock();
	}
	Refresh();
}

void JoystickCanvas::setYValue(int16_t y) {
	if(isLeftJoystick) {
		inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::LEFT_Y, y);
		correctForCircleLock();
	} else {
		inputInstance->triggerNumberValuesJoystick(ControllerNumberValues::RIGHT_Y, y);
		correctForCircleLock();
	}
	Refresh();
}

BottomUI::BottomUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, DataProcessing* input, std::shared_ptr<ProjectHandler> projHandler) {
	// TODO set up joysticks
	buttonData     = buttons;
	mainSettings   = settings;
	projectHandler = projHandler;

	inputInstance = input;

	parent = parentFrame;

	mainSizer          = new wxBoxSizer(wxVERTICAL);
	horizontalBoxSizer = new wxBoxSizer(wxHORIZONTAL);

	leftJoystickDrawer  = new JoystickCanvas(settings, parentFrame, inputInstance, true);
	rightJoystickDrawer = new JoystickCanvas(settings, parentFrame, inputInstance, false);

	leftJoystickDrawer->SetToolTip("Edit left joystick value");
	rightJoystickDrawer->SetToolTip("Edit right joystick value");

	leftJoystickDrawer->getLockButton()->Bind(wxEVT_BUTTON, &BottomUI::onLeftJoystickLock, this);
	rightJoystickDrawer->getLockButton()->Bind(wxEVT_BUTTON, &BottomUI::onRightJoystickLock, this);

	leftJoystickDrawer->getLockButton()->SetToolTip("Set current value of left gamepad joystick to frame");
	rightJoystickDrawer->getLockButton()->SetToolTip("Set current value of right gamepad joystick to frame");

	// Not shown by default
	motionAndTouchWidget = new MotionAndTouchWidget(parentFrame, inputInstance);

	wxSize gridSize;
	// Just to get a rough estimate
	int w = ButtonData::KeyWidth * buttonData->buttonMapping[Btn::A]->resizedGridOffBitmap->GetWidth();
	int h = ButtonData::KeyHeight * buttonData->buttonMapping[Btn::A]->resizedGridOffBitmap->GetHeight();
	gridSize.SetWidth(w);
	gridSize.SetHeight(h);

	buttonGrid = new ButtonGrid(parentFrame, gridSize, buttonData, inputInstance);

	buttonGrid->SetToolTip("Toggle buttons for frames with mouse");

	frameViewerCanvas = new FrameViewerCanvas(parentFrame, new wxBitmap(HELPERS::resolvePath((*mainSettings)["videoViewerDefaultImage"].GetString()), wxBITMAP_TYPE_JPEG));

	frameViewerCanvas->SetToolTip("View current frame's screenshot");

	inputInstance->setInputCallback(std::bind(&BottomUI::refreshDataViews, this, std::placeholders::_1));

	// Add the joystick submenu
	joystickSubMenu = new wxMenu();
	joystickSubMenu->Bind(wxEVT_MENU_OPEN, &BottomUI::onJoystickMenuOpen, this);
	currentJoyDefined = false;
	lastButtonState   = 0;

	// These take up much less space than the grid
	horizontalBoxSizer->Add(leftJoystickDrawer->getSizer(), 0);
	horizontalBoxSizer->Add(rightJoystickDrawer->getSizer(), 0);
	horizontalBoxSizer->Add(buttonGrid, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_VERTICAL);

	frameViewerCanvas->SetMinSize(wxSize(0, 0));

	// Proportion HAS to be zero here, it's a requirment
	mainSizer->Add(frameViewerCanvas, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(horizontalBoxSizer, 0, wxEXPAND | wxALL);

	theGrid->Add(mainSizer, 1, wxEXPAND | wxALL);

	wxAcceleratorEntry entries[1];

	screenshotExportID = wxNewId();

	entries[0].Set(wxACCEL_CTRL, (int)'E', screenshotExportID, editMenu.Append(screenshotExportID, wxT("Export Screenshot\tCtrl+E")));

	wxAcceleratorTable accel(11, entries);
	frameViewerCanvas->SetAcceleratorTable(accel);

	frameViewerCanvas->Bind(wxEVT_CONTEXT_MENU, &BottomUI::onFrameViewerRightClick, this);

	frameViewerCanvas->Bind(wxEVT_MENU, &BottomUI::exportImageView, this, screenshotExportID);
}

void BottomUI::refreshDataViews(uint8_t refreshFramebuffer) {
	// Just refresh the grid and the joysticks
	leftJoystickDrawer->Refresh();
	rightJoystickDrawer->Refresh();

	// TODO update motion values in motion and touch editor
	motionAndTouchWidget->updateMotionValues();

	buttonGrid->Refresh();

	if(refreshFramebuffer) {
		// Check to see if framebuffer is avaliable to draw
		wxFileName framebufferFileName = inputInstance->getFramebufferPathForCurrentFramebuf();
		if(framebufferFileName.FileExists()) {
			wxImage framebuf(framebufferFileName.GetFullPath(), wxBITMAP_TYPE_JPEG);
			frameViewerCanvas->setPrimaryBitmap(new wxBitmap(framebuf));
		} else {
			// Go back to default
			frameViewerCanvas->setPrimaryBitmap(nullptr);
		}
	}
}

void BottomUI::recieveGameFramebuffer(std::vector<uint8_t> jpegBuffer) {
	frameViewerCanvas->setPrimaryBitmap(new wxBitmap(HELPERS::getImageFromJPEGData(jpegBuffer)));
}

void BottomUI::onFrameViewerRightClick(wxContextMenuEvent& event) {
	const wxPoint mousePosition = frameViewerCanvas->ScreenToClient(event.GetPosition());

	frameViewerCanvas->PopupMenu(&editMenu, mousePosition);
}

void BottomUI::exportImageView(wxCommandEvent& event) {
	// https://forums.wxwidgets.org/viewtopic.php?p=32313#32313
	// Save a screenshot of the frame view if the user wants to do something with it

	wxClientDC dcWindow(frameViewerCanvas);

	wxCoord windowWidth, windowHeight;
	dcWindow.GetSize(&windowWidth, &windowHeight);

	wxBitmap screenshot(windowWidth, windowHeight, wxBITMAP_SCREEN_DEPTH);

	wxMemoryDC memDC;

	memDC.SelectObject(screenshot);
	memDC.Blit(0, 0, windowWidth, windowHeight, &dcWindow, 0, 0);
	memDC.SelectObject(wxNullBitmap);

	wxFileName imageLocation = projectHandler->getProjectStart();
	imageLocation.AppendDir("exported_images");
	imageLocation.SetExt("png");

	uint32_t exportIndex = 0;
	while(true) {
		imageLocation.SetName(wxString::Format("exported_image_%hu", exportIndex));

		if(!imageLocation.FileExists()) {
			break;
		} else {
			exportIndex++;
		}
	}

	imageLocation.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	screenshot.SaveFile(imageLocation.GetFullPath(), wxBITMAP_TYPE_PNG);
}

void BottomUI::onJoystickSelect(wxCommandEvent& event) {
	// There is a menu element that says no joysticks are defined
	// If clicked on, this will not fire
	if(joysticksExist) {
		if(currentJoyDefined) {
			// Disable the earlier one
			currentJoy->ReleaseCapture();
			delete currentJoy;
			currentJoyDefined = false;
		}
		// Joystick selected, get the index to know which joy
		currentJoy = new wxJoystick(event.GetId() - joystickSubmenuIDBase);
		// Prepare mapping
		joyButtonToSwitch.clear();
		povToSwitch.clear();
		axisButtonsToSwitch.clear();
		leftStickAxis.clear();
		rightStickAxis.clear();
		axisDirection.clear();
		lastState.clear();
		// Get mapping from hex string
		wxString hexWxString  = getJoyHexString(currentJoy);
		const char* hexString = hexWxString.ToUTF8().data();
		std::string joyMapping;
		// Check if exists and use default if not
		if((*mainSettings)["joystickMappings"].HasMember(hexString)) {
			joyMapping = (*mainSettings)["joystickMappings"][hexString].GetString();
		} else {
			// This default is for the pro controller connected to pc without drivers
			joyMapping = (*mainSettings)["joystickMappings"]["0x45E,0x28E"].GetString();
		}
		// Break it apart and do things
		std::vector<std::string> parts = HELPERS::splitString(joyMapping, ',');
		for(auto const& part : parts) {
			std::vector<std::string> inputParts = HELPERS::splitString(part, ':');
			char firstChar                      = inputParts[1].front();
			int index;
			if(firstChar == 'b') {
				// It adds one for some reason, ask SDL
				index = strtol(inputParts[1].erase(0, 1).c_str(), nullptr, 10) - 1;
				// This is a button
				if(buttonData->stringToButton.count(inputParts[0])) {
					// It's a normal button
					joyButtonToSwitch[index] = (int)buttonData->stringToButton[inputParts[0]];
				} else {
					// It's extended
					// Go beyond the end of Btn
					joyButtonToSwitch[index] = stringToButtonExtended[inputParts[0]] + Btn::BUTTONS_SIZE;
				}
			} else if(firstChar == 'h') {
				// This is a hat, usually pov
				// Hat comprises of two parts, but basically ignore the first part
				// Because there is usually just one hat
				// The charactr is a bit weird, translate it
				// #define 	SDL_HAT_UP   0x01
				// #define 	SDL_HAT_RIGHT   0x02
				// #define 	SDL_HAT_DOWN   0x04
				// #define 	SDL_HAT_LEFT   0x08
				char indexChar = inputParts[1].at(3);
				if(indexChar == '1') {
					index = 0;
				} else if(indexChar == '2') {
					index = 1;
				} else if(indexChar == '4') {
					index = 2;
				} else if(indexChar == '8') {
					index = 3;
				}
				// wxWidgets just does this https://docs.wxwidgets.org/3.0/classwx_joystick.html#a10712042f8cbca788ef04e96eab375a4
				if(buttonData->stringToButton.count(inputParts[0])) {
					// It's a normal button
					povToSwitch[index] = (int)buttonData->stringToButton[inputParts[0]];
				} else {
					// It's extended
					// Go beyond the end of Btn
					povToSwitch[index] = stringToButtonExtended[inputParts[0]] + Btn::BUTTONS_SIZE;
				}
			} else if(firstChar == 'a') {
				// This is an axis
				index = strtol(inputParts[1].erase(0, 1).c_str(), nullptr, 10);
				// Whether to flip the axis or not
				bool direction       = inputParts[1].at(1) == '+';
				axisDirection[index] = direction;
				// This is a button
				if(buttonData->stringToButton.count(inputParts[0])) {
					// It's a normal button
					// Add to axisButtons, as they are read continuously, unlike the other axis
					axisButtonsToSwitch[index] = (int)buttonData->stringToButton[inputParts[0]];
				} else {
					// It's extended
					// Add each kind to a separate map
					if(inputParts[0] == "LSX" || inputParts[0] == "LSY") {
						leftStickAxis[index] = stringToButtonExtended[inputParts[0]];
					} else if(inputParts[0] == "RSX" || inputParts[0] == "RSY") {
						rightStickAxis[index] = stringToButtonExtended[inputParts[0]];
					}
				}
			}
		}
		// Finally, start listening
		currentJoy->SetCapture(parent);
		currentJoyDefined = true;
	}
}

void BottomUI::onJoystickMenuOpen(wxMenuEvent& event) {
	// Remove all current items
	int currentNumOfItems = joystickSubMenu->GetMenuItemCount();
	for(int i = 0; i < currentNumOfItems; i++) {
		joystickSubMenu->Delete(i + joystickSubmenuIDBase);
	}
	// Get number of currently selected joysticks and list them
	int numOfConnectedJoysticks = wxJoystick::GetNumberJoysticks();
	if(numOfConnectedJoysticks == 0) {
		joysticksExist = false;
		joystickSubMenu->Append(joystickSubmenuIDBase, "No Joysticks Connected");
	} else {
		joysticksExist = true;
		for(int i = 0; i < numOfConnectedJoysticks; i++) {
			// Briefly open it for info
			// Not using wxJOYSTICK1 or wxJOYSTICK2, because that restricts me to two
			wxJoystick* joy = new wxJoystick(i);
			// wxString name   = joy->GetProductName();
			wxString printString = getJoyHexString(joy);
			// TODO for the name, use this https://forums.wxwidgets.org/viewtopic.php?f=1&t=46931&p=197440#p197459
			joystickSubMenu->Append(i + joystickSubmenuIDBase, printString);
			// Delete it because it doesn't need to be used right now
			delete joy;
		}
	}
}

wxString BottomUI::getJoyHexString(wxJoystick* joy) {
	int manufacturerID = joy->GetManufacturerId();
	int productID      = joy->GetProductId();
	// Convert each to a hex string using sprintf, to save on bloat from stringstream
	char buffer[10];
	sprintf(buffer, "0x%X,0x%X", manufacturerID, productID);
	return wxString(buffer);
}

void BottomUI::listenToJoystick() {
	if(currentJoyDefined) {
		// Handle joystick events live
		// Will be really complicated, so use this https://robsears.com/ultimate-wxjoystick-tutorial/
		// Use https://github.com/gabomdq/gamecontrollerdb for mapping
		// Check this too https://gaming.stackexchange.com/a/358794
		// Button down event
		// Check POV too just in case
		int buttonState = currentJoy->GetButtonState();
		for(int i = 0; i < currentJoy->GetNumberButtons(); i++) {
			// Get the value with bitwise
			uint8_t isPressed = GET_BIT(buttonState, i);
			if(isPressed && !GET_BIT(lastButtonState, i)) {
				// This button wasn't clicked before and now it is, trigger it
				// Essentially, it's a button down event for this button
				if(joyButtonToSwitch.count(i)) {
					Btn button = (Btn)joyButtonToSwitch[i];
					inputInstance->triggerButton(button);
				}
			}
		}
		lastButtonState = buttonState;

		// Move event, I think any axis
		// Check each axis individually
		for(int i = 0; i < currentJoy->GetNumberAxes(); i++) {
			// Range should be from -32768 to +32768 or 0 to +65535
			// Ranges should be the same for all axis
			// Automatically floors
			int axisMiddle = (currentJoy->GetXMin() + currentJoy->GetXMax()) / 2;
			int axisValue  = currentJoy->GetPosition(i);

			if(axisButtonsToSwitch.count(i)) {
				int switchID = axisButtonsToSwitch[i];
				// if(switchID < Btn::BUTTONS_SIZE) {
				// Check RS and LS only because it's susceptible to this
				if((Btn)switchID == Btn::ZL || (Btn)switchID == Btn::ZR) {
					// Flip if needed
					if(axisDirection[i] ? axisValue < axisMiddle : axisValue > axisMiddle) {
						// Trigger ZL
						if(lastState[Btn::ZL] != true) {
							inputInstance->triggerButton(Btn::ZL);
							lastState[Btn::ZL] = true;
						}
						// ZR is now off
						lastState[Btn::ZR] = false;
					} else if(axisDirection[i] ? axisValue > axisMiddle : axisValue < axisMiddle) {
						// Trigger ZR
						if(lastState[Btn::ZR] != true) {
							inputInstance->triggerButton(Btn::ZR);
							lastState[Btn::ZR] = true;
						}
						// ZL is now off
						lastState[Btn::ZL] = false;
					} else {
						// Both are off
						lastState[Btn::ZL] = false;
						lastState[Btn::ZR] = false;
					}
				}
			}
		}
		// I don't know what event it falls under
		// https://docs.wxwidgets.org/3.0/classwx_joystick.html#a10712042f8cbca788ef04e96eab375a4
		int pov = currentJoy->GetPOVPosition();
		if(pov != -1) {
			int povValue = pov / 9000;
			if(povToSwitch.count(povValue)) {
				int switchID = povToSwitch[povValue];
				if(povLastState != povValue) {
					if(switchID < Btn::BUTTONS_SIZE) {
						// Normal button
						// Won't check for axis, too scared
						Btn button = (Btn)switchID;
						inputInstance->triggerButton(button);
					}
					povLastState = povValue;
				}
			}
		}
	}
}

void BottomUI::onLeftJoystickLock(wxCommandEvent& event) {
	if(currentJoyDefined) {
		for(int i = 0; i < currentJoy->GetNumberAxes(); i++) {
			// Range should be from -32768 to +32768 or 0 to +65535
			// Ranges should be the same for all axis
			int axisMin = currentJoy->GetXMin();
			int axisMax = currentJoy->GetXMax();
			// Automatically floors
			int axisMiddle = (axisMin + axisMax) / 2;
			int axisValue  = currentJoy->GetPosition(i);

			// Normalize to 1, then multiply by the range
			int16_t normalizedAxisValue = ((float)(axisValue - axisMiddle) / (float)(axisMax - axisMiddle)) * ButtonData::axisMax;

			// Flip if needed
			if(!axisDirection[i]) {
				normalizedAxisValue *= -1;
			}

			if(leftStickAxis.count(i)) {
				// This is the right axis
				int axisID = leftStickAxis[i];
				if(axisID == 0) {
					// LSX
					leftJoystickDrawer->setXValue(normalizedAxisValue);
				} else if(axisID == 1) {
					// LSY
					leftJoystickDrawer->setYValue(normalizedAxisValue);
				}
			}
		}
	}
}

void BottomUI::onRightJoystickLock(wxCommandEvent& event) {
	if(currentJoyDefined) {
		for(int i = 0; i < currentJoy->GetNumberAxes(); i++) {
			// Range should be from -32768 to +32768 or 0 to +65535
			// Ranges should be the same for all axis
			int axisMin = currentJoy->GetXMin();
			int axisMax = currentJoy->GetXMax();
			// Automatically floors
			int axisMiddle = (axisMin + axisMax) / 2;
			int axisValue  = currentJoy->GetPosition(i);

			// Normalize to 1, then multiply by the range
			int16_t normalizedAxisValue = ((float)(axisValue - axisMiddle) / (float)(axisMax - axisMiddle)) * ButtonData::axisMax;

			// Flip if needed
			if(!axisDirection[i]) {
				normalizedAxisValue *= -1;
			}

			if(rightStickAxis.count(i)) {
				// This is the right axis
				int axisID = rightStickAxis[i];
				if(axisID == 2) {
					// RSX
					rightJoystickDrawer->setXValue(normalizedAxisValue);
				} else if(axisID == 3) {
					// RSY
					rightJoystickDrawer->setYValue(normalizedAxisValue);
				}
			}
		}
	}
}