#include "bottomUI.hpp"

ExtraInputMethods::ExtraInputMethods(wxFrame* parentFrame, DataProcessing* input, std::shared_ptr<ButtonData> data)
	: wxFrame(parentFrame, wxID_ANY, "Motion and Touch Editor", wxDefaultPosition, wxSize(800, 400), wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT) {
	inputInstance = input;
	buttonData    = data;

	// Start hidden
	Hide();

	mainSizer = new wxBoxSizer(wxHORIZONTAL);

	motionLeftSizer          = new wxStaticBoxSizer(wxVERTICAL, this, "Left Joycon Motion");
	accelLeftSizer           = new wxStaticBoxSizer(wxHORIZONTAL, motionLeftSizer->GetStaticBox(), "Acceleration");
	angularVelocityLeftSizer = new wxStaticBoxSizer(wxHORIZONTAL, motionLeftSizer->GetStaticBox(), "Angular Velocity");
	angleLeftSizer           = new wxStaticBoxSizer(wxHORIZONTAL, motionLeftSizer->GetStaticBox(), "Angle");
	directionXLeftSizer      = new wxStaticBoxSizer(wxHORIZONTAL, motionLeftSizer->GetStaticBox(), "Acceleration");
	directionYLeftSizer      = new wxStaticBoxSizer(wxHORIZONTAL, motionLeftSizer->GetStaticBox(), "Angular Velocity");
	directionZLeftSizer      = new wxStaticBoxSizer(wxHORIZONTAL, motionLeftSizer->GetStaticBox(), "Angle");

	accelXLeftCtrl           = new wxSpinCtrlDouble(accelLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0, 0.0);
	accelYLeftCtrl           = new wxSpinCtrlDouble(accelLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0, 0.0);
	accelZLeftCtrl           = new wxSpinCtrlDouble(accelLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0, -1.0);
	angularVelocityXLeftCtrl = new wxSpinCtrlDouble(angularVelocityLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0, 0.0);
	angularVelocityYLeftCtrl = new wxSpinCtrlDouble(angularVelocityLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0, 0.0);
	angularVelocityZLeftCtrl = new wxSpinCtrlDouble(angularVelocityLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0, 0.0);
	angleXLeftCtrl           = new wxSpinCtrlDouble(angleLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0, 0.0);
	angleYLeftCtrl           = new wxSpinCtrlDouble(angleLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0, 0.0);
	angleZLeftCtrl           = new wxSpinCtrlDouble(angleLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0, 0.0);
	directionXXLeftCtrl      = new wxSpinCtrlDouble(directionXLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 1.0);
	directionXYLeftCtrl      = new wxSpinCtrlDouble(directionXLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionXZLeftCtrl      = new wxSpinCtrlDouble(directionXLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionYXLeftCtrl      = new wxSpinCtrlDouble(directionYLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionYYLeftCtrl      = new wxSpinCtrlDouble(directionYLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 1.0);
	directionYZLeftCtrl      = new wxSpinCtrlDouble(directionYLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionZXLeftCtrl      = new wxSpinCtrlDouble(directionZLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionZYLeftCtrl      = new wxSpinCtrlDouble(directionZLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionZZLeftCtrl      = new wxSpinCtrlDouble(directionZLeftSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 1.0);

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
	directionXXLeftCtrl->SetDigits(FLT_DIG);
	directionXYLeftCtrl->SetDigits(FLT_DIG);
	directionXZLeftCtrl->SetDigits(FLT_DIG);
	directionYXLeftCtrl->SetDigits(FLT_DIG);
	directionYYLeftCtrl->SetDigits(FLT_DIG);
	directionYZLeftCtrl->SetDigits(FLT_DIG);
	directionZXLeftCtrl->SetDigits(FLT_DIG);
	directionZYLeftCtrl->SetDigits(FLT_DIG);
	directionZZLeftCtrl->SetDigits(FLT_DIG);

	accelXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	accelYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	accelZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angularVelocityXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angularVelocityYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angularVelocityZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angleXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angleYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angleZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionXXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionXYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionXZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionYXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionYYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionYZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionZXLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionZYLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionZZLeftCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);

	accelXLeftCtrl->SetToolTip("Set acceleration X value for left joycon");
	accelYLeftCtrl->SetToolTip("Set acceleration Y value for left joycon");
	accelZLeftCtrl->SetToolTip("Set acceleration Z value for left joycon");
	angularVelocityXLeftCtrl->SetToolTip("Set angular velocity X value for left joycon");
	angularVelocityYLeftCtrl->SetToolTip("Set angular velocity Y value for left joycon");
	angularVelocityZLeftCtrl->SetToolTip("Set angular velocity Z value for left joycon");
	angleXLeftCtrl->SetToolTip("Set angle X value for left joycon");
	angleYLeftCtrl->SetToolTip("Set angle Y value for left joycon");
	angleZLeftCtrl->SetToolTip("Set angle Z value for left joycon");
	directionXXLeftCtrl->SetToolTip("Set direction quaternion value XX for left joycon");
	directionXYLeftCtrl->SetToolTip("Set direction quaternion value XY for left joycon");
	directionXZLeftCtrl->SetToolTip("Set direction quaternion value XZ for left joycon");
	directionYXLeftCtrl->SetToolTip("Set direction quaternion value YX for left joycon");
	directionYYLeftCtrl->SetToolTip("Set direction quaternion value YY for left joycon");
	directionYZLeftCtrl->SetToolTip("Set direction quaternion value YZ for left joycon");
	directionZXLeftCtrl->SetToolTip("Set direction quaternion value ZX for left joycon");
	directionZYLeftCtrl->SetToolTip("Set direction quaternion value ZY for left joycon");
	directionZZLeftCtrl->SetToolTip("Set direction quaternion value ZZ for left joycon");

	accelLeftSizer->Add(accelXLeftCtrl, 0, wxEXPAND);
	accelLeftSizer->Add(accelYLeftCtrl, 0, wxEXPAND);
	accelLeftSizer->Add(accelZLeftCtrl, 0, wxEXPAND);
	angularVelocityLeftSizer->Add(angularVelocityXLeftCtrl, 0, wxEXPAND);
	angularVelocityLeftSizer->Add(angularVelocityYLeftCtrl, 0, wxEXPAND);
	angularVelocityLeftSizer->Add(angularVelocityZLeftCtrl, 0, wxEXPAND);
	angleLeftSizer->Add(angleXLeftCtrl, 0, wxEXPAND);
	angleLeftSizer->Add(angleYLeftCtrl, 0, wxEXPAND);
	angleLeftSizer->Add(angleZLeftCtrl, 0, wxEXPAND);
	directionXLeftSizer->Add(directionXXLeftCtrl, 0, wxEXPAND);
	directionXLeftSizer->Add(directionXYLeftCtrl, 0, wxEXPAND);
	directionXLeftSizer->Add(directionXZLeftCtrl, 0, wxEXPAND);
	directionYLeftSizer->Add(directionYXLeftCtrl, 0, wxEXPAND);
	directionYLeftSizer->Add(directionYYLeftCtrl, 0, wxEXPAND);
	directionYLeftSizer->Add(directionYZLeftCtrl, 0, wxEXPAND);
	directionZLeftSizer->Add(directionZXLeftCtrl, 0, wxEXPAND);
	directionZLeftSizer->Add(directionZYLeftCtrl, 0, wxEXPAND);
	directionZLeftSizer->Add(directionZZLeftCtrl, 0, wxEXPAND);

	motionLeftSizer->Add(accelLeftSizer, 1, wxEXPAND);
	motionLeftSizer->Add(angularVelocityLeftSizer, 1, wxEXPAND);
	motionLeftSizer->Add(angleLeftSizer, 1, wxEXPAND);
	motionLeftSizer->Add(directionXLeftSizer, 1, wxEXPAND);
	motionLeftSizer->Add(directionYLeftSizer, 1, wxEXPAND);
	motionLeftSizer->Add(directionZLeftSizer, 1, wxEXPAND);

	motionRightSizer          = new wxStaticBoxSizer(wxVERTICAL, this, "Right Joycon Motion");
	accelRightSizer           = new wxStaticBoxSizer(wxHORIZONTAL, motionRightSizer->GetStaticBox(), "Acceleration");
	angularVelocityRightSizer = new wxStaticBoxSizer(wxHORIZONTAL, motionRightSizer->GetStaticBox(), "Angular Velocity");
	angleRightSizer           = new wxStaticBoxSizer(wxHORIZONTAL, motionRightSizer->GetStaticBox(), "Angle");
	directionXRightSizer      = new wxStaticBoxSizer(wxHORIZONTAL, motionRightSizer->GetStaticBox(), "Acceleration");
	directionYRightSizer      = new wxStaticBoxSizer(wxHORIZONTAL, motionRightSizer->GetStaticBox(), "Angular Velocity");
	directionZRightSizer      = new wxStaticBoxSizer(wxHORIZONTAL, motionRightSizer->GetStaticBox(), "Angle");

	accelXRightCtrl           = new wxSpinCtrlDouble(accelRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0, 0.0);
	accelYRightCtrl           = new wxSpinCtrlDouble(accelRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0, 0.0);
	accelZRightCtrl           = new wxSpinCtrlDouble(accelRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -7.0, 7.0, -1.0);
	angularVelocityXRightCtrl = new wxSpinCtrlDouble(angularVelocityRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0, 0.0);
	angularVelocityYRightCtrl = new wxSpinCtrlDouble(angularVelocityRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0, 0.0);
	angularVelocityZRightCtrl = new wxSpinCtrlDouble(angularVelocityRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -5.0, 5.0, 0.0);
	angleXRightCtrl           = new wxSpinCtrlDouble(angleRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0, 0.0);
	angleYRightCtrl           = new wxSpinCtrlDouble(angleRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0, 0.0);
	angleZRightCtrl           = new wxSpinCtrlDouble(angleRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1.0, 1.0, 0.0);
	directionXXRightCtrl      = new wxSpinCtrlDouble(directionXRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 1.0);
	directionXYRightCtrl      = new wxSpinCtrlDouble(directionXRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionXZRightCtrl      = new wxSpinCtrlDouble(directionXRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionYXRightCtrl      = new wxSpinCtrlDouble(directionYRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionYYRightCtrl      = new wxSpinCtrlDouble(directionYRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 1.0);
	directionYZRightCtrl      = new wxSpinCtrlDouble(directionYRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionZXRightCtrl      = new wxSpinCtrlDouble(directionZRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionZYRightCtrl      = new wxSpinCtrlDouble(directionZRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 0.0);
	directionZZRightCtrl      = new wxSpinCtrlDouble(directionZRightSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.0, 1.0, 1.0);

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
	directionXXRightCtrl->SetDigits(FLT_DIG);
	directionXYRightCtrl->SetDigits(FLT_DIG);
	directionXZRightCtrl->SetDigits(FLT_DIG);
	directionYXRightCtrl->SetDigits(FLT_DIG);
	directionYYRightCtrl->SetDigits(FLT_DIG);
	directionYZRightCtrl->SetDigits(FLT_DIG);
	directionZXRightCtrl->SetDigits(FLT_DIG);
	directionZYRightCtrl->SetDigits(FLT_DIG);
	directionZZRightCtrl->SetDigits(FLT_DIG);

	accelXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	accelYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	accelZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angularVelocityXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angularVelocityYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angularVelocityZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angleXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angleYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	angleZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionXXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionXYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionXZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionYXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionYYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionYZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionZXRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionZYRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);
	directionZZRightCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &ExtraInputMethods::motionValueChanged, this);

	accelXRightCtrl->SetToolTip("Set acceleration X value for right joycon");
	accelYRightCtrl->SetToolTip("Set acceleration Y value for right joycon");
	accelZRightCtrl->SetToolTip("Set acceleration Z value for right joycon");
	angularVelocityXRightCtrl->SetToolTip("Set angular velocity X value for right joycon");
	angularVelocityYRightCtrl->SetToolTip("Set angular velocity Y value for right joycon");
	angularVelocityZRightCtrl->SetToolTip("Set angular velocity Z value for right joycon");
	angleXRightCtrl->SetToolTip("Set angle X value for right joycon");
	angleYRightCtrl->SetToolTip("Set angle Y value for right joycon");
	angleZRightCtrl->SetToolTip("Set angle Z value for right joycon");
	directionXXRightCtrl->SetToolTip("Set direction quaternion value XX for right joycon");
	directionXYRightCtrl->SetToolTip("Set direction quaternion value XY for right joycon");
	directionXZRightCtrl->SetToolTip("Set direction quaternion value XZ for right joycon");
	directionYXRightCtrl->SetToolTip("Set direction quaternion value YX for right joycon");
	directionYYRightCtrl->SetToolTip("Set direction quaternion value YY for right joycon");
	directionYZRightCtrl->SetToolTip("Set direction quaternion value YZ for right joycon");
	directionZXRightCtrl->SetToolTip("Set direction quaternion value ZX for right joycon");
	directionZYRightCtrl->SetToolTip("Set direction quaternion value ZY for right joycon");
	directionZZRightCtrl->SetToolTip("Set direction quaternion value ZZ for right joycon");

	accelRightSizer->Add(accelXRightCtrl, 0, wxEXPAND);
	accelRightSizer->Add(accelYRightCtrl, 0, wxEXPAND);
	accelRightSizer->Add(accelZRightCtrl, 0, wxEXPAND);
	angularVelocityRightSizer->Add(angularVelocityXRightCtrl, 0, wxEXPAND);
	angularVelocityRightSizer->Add(angularVelocityYRightCtrl, 0, wxEXPAND);
	angularVelocityRightSizer->Add(angularVelocityZRightCtrl, 0, wxEXPAND);
	angleRightSizer->Add(angleXRightCtrl, 0, wxEXPAND);
	angleRightSizer->Add(angleYRightCtrl, 0, wxEXPAND);
	angleRightSizer->Add(angleZRightCtrl, 0, wxEXPAND);
	directionXRightSizer->Add(directionXXRightCtrl, 0, wxEXPAND);
	directionXRightSizer->Add(directionXYRightCtrl, 0, wxEXPAND);
	directionXRightSizer->Add(directionXZRightCtrl, 0, wxEXPAND);
	directionYRightSizer->Add(directionYXRightCtrl, 0, wxEXPAND);
	directionYRightSizer->Add(directionYYRightCtrl, 0, wxEXPAND);
	directionYRightSizer->Add(directionYZRightCtrl, 0, wxEXPAND);
	directionZRightSizer->Add(directionZXRightCtrl, 0, wxEXPAND);
	directionZRightSizer->Add(directionZYRightCtrl, 0, wxEXPAND);
	directionZRightSizer->Add(directionZZRightCtrl, 0, wxEXPAND);

	motionRightSizer->Add(accelRightSizer, 1, wxEXPAND);
	motionRightSizer->Add(angularVelocityRightSizer, 1, wxEXPAND);
	motionRightSizer->Add(angleRightSizer, 1, wxEXPAND);
	motionRightSizer->Add(directionXRightSizer, 1, wxEXPAND);
	motionRightSizer->Add(directionYRightSizer, 1, wxEXPAND);
	motionRightSizer->Add(directionZRightSizer, 1, wxEXPAND);

	wxBoxSizer* touchAndMouseSizer = new wxBoxSizer(wxVERTICAL);

	mainTouchSizer   = new wxStaticBoxSizer(wxVERTICAL, this, "Touchscreen");
	firstTouchSizer  = new wxStaticBoxSizer(wxVERTICAL, mainTouchSizer->GetStaticBox(), "First Touch");
	secondTouchSizer = new wxStaticBoxSizer(wxVERTICAL, mainTouchSizer->GetStaticBox(), "Second Touch");
	touchesSizer     = new wxBoxSizer(wxHORIZONTAL);

	// clang-format off
	touchX1Ctrl = new wxSpinCtrl(firstTouchSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1279, 1139);
	touchY1Ctrl = new wxSpinCtrl(firstTouchSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 719, 359);
	touchX2Ctrl = new wxSpinCtrl(secondTouchSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1279, 1139);
	touchY2Ctrl = new wxSpinCtrl(secondTouchSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 719, 359);
	numberOfTouchesCtrl = new wxSpinCtrl(mainTouchSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 2, 0);
	// clang-format on

	touchX1Ctrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::touchValueChanged, this);
	touchY1Ctrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::touchValueChanged, this);
	touchX2Ctrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::touchValueChanged, this);
	touchY2Ctrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::touchValueChanged, this);
	numberOfTouchesCtrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::numOfTouchesChanged, this);

	touchX1Ctrl->SetToolTip("Set coordinate X for touch #1");
	touchY1Ctrl->SetToolTip("Set coordinate Y for touch #1");
	touchX2Ctrl->SetToolTip("Set coordinate X for touch #2");
	touchY2Ctrl->SetToolTip("Set coordinate Y for touch #2");
	numberOfTouchesCtrl->SetToolTip("Set number of touches to be sent to the game");

	firstTouchSizer->Add(touchX1Ctrl, 0, wxEXPAND);
	firstTouchSizer->Add(touchY1Ctrl, 0, wxEXPAND);
	secondTouchSizer->Add(touchX2Ctrl, 0, wxEXPAND);
	secondTouchSizer->Add(touchY2Ctrl, 0, wxEXPAND);

	touchesSizer->Add(firstTouchSizer, 1, wxEXPAND);
	touchesSizer->Add(secondTouchSizer, 1, wxEXPAND);

	mainTouchSizer->Add(touchesSizer, 1, wxEXPAND);
	mainTouchSizer->Add(numberOfTouchesCtrl, 0, wxEXPAND);

	mainMouseSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Mouse Movement");

	// clang-format off
	// I don't know any of the ranges of these values
	mouseXCtrl = new wxSpinCtrl(mainMouseSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1279, 0);
	mouseYCtrl = new wxSpinCtrl(mainMouseSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 719, 0);
	mouseVelocityXCtrl = new wxSpinCtrl(mainMouseSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1279, 0);
	mouseVelocityYCtrl = new wxSpinCtrl(mainMouseSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 719, 0);
	scrollVelocityXCtrl = new wxSpinCtrl(mainMouseSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
	scrollVelocityYCtrl = new wxSpinCtrl(mainMouseSizer->GetStaticBox(), wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
	// clang-format on

	mouseXCtrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::mouseValueChanged, this);
	mouseYCtrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::mouseValueChanged, this);
	mouseVelocityXCtrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::mouseValueChanged, this);
	mouseVelocityYCtrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::mouseValueChanged, this);
	scrollVelocityXCtrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::mouseValueChanged, this);
	scrollVelocityYCtrl->Bind(wxEVT_SPINCTRL, &ExtraInputMethods::mouseValueChanged, this);

	mouseXCtrl->SetToolTip("Set mouse coordinate X");
	mouseYCtrl->SetToolTip("Set mouse coordinate Y");
	mouseVelocityXCtrl->SetToolTip("Set mouse velocity X");
	mouseVelocityYCtrl->SetToolTip("Set mouse velocity X");
	scrollVelocityXCtrl->SetToolTip("Set mouse scroll velocity X");
	scrollVelocityYCtrl->SetToolTip("Set mouse scroll velocity X");

	keyboardKeysSizer      = new wxStaticBoxSizer(wxVERTICAL, this, "Keyboard Keys");
	keyboardModifiersSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Keyboard Modifiers");
	mouseButtonsSizer      = new wxStaticBoxSizer(wxVERTICAL, this, "Mouse Buttons");

	keyboardKeys      = new wxListCtrl(keyboardKeysSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER);
	keyboardModifiers = new wxListCtrl(keyboardModifiersSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER);
	mouseButtons      = new wxListCtrl(mouseButtonsSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER);

	keyboardKeys->AppendColumn("", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	keyboardModifiers->AppendColumn("", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	mouseButtons->AppendColumn("", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);

	for(auto const& keyboardKey : buttonData->stringToKeyboardKey) {
		wxString name                                 = wxString::FromUTF8(keyboardKey.first);
		keyboardKeyIndices[keyboardKeys->GetCount()]  = keyboardKey.second;
		keyboardKeyIndicesReverse[keyboardKey.second] = keyboardKeys->GetCount();

		wxListItem item;
		item.SetText(name);
		keyboardKeys->InsertItem(item);
	}

	for(auto const& keyboardModifier : buttonData->stringToKeyboardModifier) {
		wxString name                                           = wxString::FromUTF8(keyboardModifier.first);
		keyboardModifierIndices[keyboardModifiers->GetCount()]  = keyboardModifier.second;
		keyboardModifierIndicesReverse[keyboardModifier.second] = keyboardModifiers->GetCount();

		wxListItem item;
		item.SetText(name);
		keyboardModifiers->InsertItem(item);
	}

	for(auto const& mouseButton : buttonData->stringToMouseButton) {
		wxString name                                 = wxString::FromUTF8(mouseButton.first);
		mouseButtonIndices[mouseButtons->GetCount()]  = mouseButton.second;
		mouseButtonIndicesReverse[mouseButton.second] = mouseButtons->GetCount();

		wxListItem item;
		item.SetText(name);
		mouseButtons->InsertItem(item);
	}

	keyboardKeys->Bind(wxEVT_LIST_ITEM_SELECTED, &ExtraInputMethods::keyboardKeysChanged, this);
	keyboardModifiers->Bind(wxEVT_LIST_ITEM_SELECTED, &ExtraInputMethods::keyboardModifiersChanged, this);
	mouseButtons->Bind(wxEVT_LIST_ITEM_SELECTED, &ExtraInputMethods::mouseButtonsChanged, this);

	keyboardKeysSizer->Add(keyboardKeys, 1, wxEXPAND);
	keyboardModifiersSizer->Add(keyboardModifiers, 1, wxEXPAND);
	mouseButtonsSizer->Add(mouseButtons, 1, wxEXPAND);

	mainMouseSizer->Add(mouseXCtrl, 0, wxEXPAND);
	mainMouseSizer->Add(mouseYCtrl, 0, wxEXPAND);
	mainMouseSizer->Add(mouseVelocityXCtrl, 0, wxEXPAND);
	mainMouseSizer->Add(mouseVelocityYCtrl, 0, wxEXPAND);
	mainMouseSizer->Add(scrollVelocityXCtrl, 0, wxEXPAND);
	mainMouseSizer->Add(scrollVelocityYCtrl, 0, wxEXPAND);

	touchAndMouseSizer->Add(mainTouchSizer, 0, wxEXPAND);
	touchAndMouseSizer->Add(mainMouseSizer, 0, wxEXPAND);

	mainSizer->Add(motionLeftSizer, 0, wxEXPAND);
	mainSizer->Add(motionRightSizer, 0, wxEXPAND);
	mainSizer->Add(touchAndMouseSizer, 0, wxEXPAND);
	mainSizer->Add(keyboardKeysSizer, 0, wxEXPAND);
	mainSizer->Add(keyboardModifiersSizer, 0, wxEXPAND);
	mainSizer->Add(mouseButtonsSizer, 0, wxEXPAND);

	Bind(wxEVT_CLOSE_WINDOW, &ExtraInputMethods::onClose, this);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void ExtraInputMethods::motionValueChanged(wxSpinDoubleEvent& event) {
	using CNV = ControllerNumberValues;

	// TODO use unordered_map here
	wxSpinCtrlDouble* widget = (wxSpinCtrlDouble*)event.GetEventObject();
	if(widget == accelXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ACCEL_X_LEFT, (float)widget->GetValue());
	} else if(widget == accelYLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ACCEL_Y_LEFT, (float)widget->GetValue());
	} else if(widget == accelZLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ACCEL_Z_LEFT, (float)widget->GetValue());
	} else if(widget == angularVelocityXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::GYRO_X_LEFT, (float)widget->GetValue());
	} else if(widget == angularVelocityYLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::GYRO_Y_LEFT, (float)widget->GetValue());
	} else if(widget == angularVelocityZLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::GYRO_Z_LEFT, (float)widget->GetValue());
	} else if(widget == angleXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ANGLE_X_LEFT, (float)widget->GetValue());
	} else if(widget == angleXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ANGLE_Y_LEFT, (float)widget->GetValue());
	} else if(widget == angleXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ANGLE_Z_LEFT, (float)widget->GetValue());
	} else if(widget == directionXXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_XX_LEFT, (float)widget->GetValue());
	} else if(widget == directionXYLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_XY_LEFT, (float)widget->GetValue());
	} else if(widget == directionXZLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_XZ_LEFT, (float)widget->GetValue());
	} else if(widget == directionYXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_YX_LEFT, (float)widget->GetValue());
	} else if(widget == directionYYLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_YY_LEFT, (float)widget->GetValue());
	} else if(widget == directionYZLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_YZ_LEFT, (float)widget->GetValue());
	} else if(widget == directionZXLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_ZX_LEFT, (float)widget->GetValue());
	} else if(widget == directionZYLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_ZY_LEFT, (float)widget->GetValue());
	} else if(widget == directionZZLeftCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_ZZ_LEFT, (float)widget->GetValue());
	} else if(widget == accelXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ACCEL_X_RIGHT, (float)widget->GetValue());
	} else if(widget == accelYRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ACCEL_Y_RIGHT, (float)widget->GetValue());
	} else if(widget == accelZRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ACCEL_Z_RIGHT, (float)widget->GetValue());
	} else if(widget == angularVelocityXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::GYRO_X_RIGHT, (float)widget->GetValue());
	} else if(widget == angularVelocityYRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::GYRO_Y_RIGHT, (float)widget->GetValue());
	} else if(widget == angularVelocityZRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::GYRO_Z_RIGHT, (float)widget->GetValue());
	} else if(widget == angleXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ANGLE_X_RIGHT, (float)widget->GetValue());
	} else if(widget == angleXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ANGLE_Y_RIGHT, (float)widget->GetValue());
	} else if(widget == angleXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::ANGLE_Z_RIGHT, (float)widget->GetValue());
	} else if(widget == directionXXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_XX_RIGHT, (float)widget->GetValue());
	} else if(widget == directionXYRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_XY_RIGHT, (float)widget->GetValue());
	} else if(widget == directionXZRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_XZ_RIGHT, (float)widget->GetValue());
	} else if(widget == directionYXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_YX_RIGHT, (float)widget->GetValue());
	} else if(widget == directionYYRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_YY_RIGHT, (float)widget->GetValue());
	} else if(widget == directionYZRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_YZ_RIGHT, (float)widget->GetValue());
	} else if(widget == directionZXRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_ZX_RIGHT, (float)widget->GetValue());
	} else if(widget == directionZYRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_ZY_RIGHT, (float)widget->GetValue());
	} else if(widget == directionZZRightCtrl) {
		inputInstance->triggerNumberValuesMotion(CNV::DIRECTION_ZZ_RIGHT, (float)widget->GetValue());
	}
}

void ExtraInputMethods::updateAllValues() {
	using CNV = ControllerNumberValues;

	uint8_t isVisible = IsVisible();
	if(isVisible || !lastWasVisible) {
		accelXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ACCEL_X_LEFT));
		accelYLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ACCEL_Y_LEFT));
		accelZLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ACCEL_Z_LEFT));
		angularVelocityXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::GYRO_X_LEFT));
		angularVelocityYLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::GYRO_Y_LEFT));
		angularVelocityZLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::GYRO_Z_LEFT));
		angleXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ANGLE_X_LEFT));
		angleXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ANGLE_Y_LEFT));
		angleXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ANGLE_Z_LEFT));
		directionXXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_XX_LEFT));
		directionXYLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_XY_LEFT));
		directionXZLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_XZ_LEFT));
		directionYXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_YX_LEFT));
		directionYYLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_YY_LEFT));
		directionYZLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_YZ_LEFT));
		directionZXLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_ZX_LEFT));
		directionZYLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_ZY_LEFT));
		directionZZLeftCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_ZZ_LEFT));

		accelXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ACCEL_X_RIGHT));
		accelYRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ACCEL_Y_RIGHT));
		accelZRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ACCEL_Z_RIGHT));
		angularVelocityXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::GYRO_X_RIGHT));
		angularVelocityYRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::GYRO_Y_RIGHT));
		angularVelocityZRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::GYRO_Z_RIGHT));
		angleXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ANGLE_X_RIGHT));
		angleXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ANGLE_Y_RIGHT));
		angleXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::ANGLE_Z_RIGHT));
		directionXXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_XX_RIGHT));
		directionXYRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_XY_RIGHT));
		directionXZRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_XZ_RIGHT));
		directionYXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_YX_RIGHT));
		directionYYRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_YY_RIGHT));
		directionYZRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_YZ_RIGHT));
		directionZXRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_ZX_RIGHT));
		directionZYRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_ZY_RIGHT));
		directionZZRightCtrl->SetValue(inputInstance->getNumberValueCurrentMotion(CNV::DIRECTION_ZZ_RIGHT));

		touchX1Ctrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::TOUCH_X_1));
		touchY1Ctrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::TOUCH_Y_1));
		touchX2Ctrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::TOUCH_X_2));
		touchY2Ctrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::TOUCH_Y_2));
		numberOfTouchesCtrl->SetValue(inputInstance->getNumberOfTouchesCurrent());

		mouseXCtrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::MOUSE_X));
		mouseYCtrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::MOUSE_Y));
		mouseVelocityXCtrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::MOUSE_VELOCITY_X));
		mouseVelocityYCtrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::MOUSE_VELOCITY_Y));
		scrollVelocityXCtrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::SCROLL_VELOCITY_X));
		scrollVelocityYCtrl->SetValue(inputInstance->getExtraValueCurrent(ExtraValues::SCROLL_VELOCITY_Y));

		for(auto const& key : buttonData->stringToKeyboardKey) {
			bool pressed = inputInstance->getKeyboardButtonCurrent(key.second);
			int index    = keyboardKeyIndicesReverse[key.second];
			if(pressed) {
				keyboardKeys->SetItemBackgroundColour(index, *wxBLUE);
			} else {
				keyboardKeys->SetItemBackgroundColour(index, *wxWHITE);
			}
		}

		for(auto const& modifier : buttonData->stringToKeyboardModifier) {
			bool pressed = inputInstance->getKeyboardModifierCurrent(modifier.second);
			int index    = keyboardModifierIndicesReverse[modifier.second];
			if(pressed) {
				keyboardModifiers->SetItemBackgroundColour(index, *wxBLUE);
			} else {
				keyboardModifiers->SetItemBackgroundColour(index, *wxWHITE);
			}
		}

		for(auto const& button : buttonData->stringToMouseButton) {
			bool pressed = inputInstance->getMouseButtonCurrent(button.second);
			int index    = mouseButtonIndicesReverse[button.second];
			if(pressed) {
				mouseButtons->SetItemBackgroundColour(index, *wxBLUE);
			} else {
				mouseButtons->SetItemBackgroundColour(index, *wxWHITE);
			}
		}
	}

	lastWasVisible = isVisible;
}

void ExtraInputMethods::touchValueChanged(wxSpinEvent& event) {
	wxSpinCtrl* widget = (wxSpinCtrl*)event.GetEventObject();
	if(widget == touchX1Ctrl) {
		inputInstance->triggerExtraValue(ExtraValues::TOUCH_X_1, (int32_t)widget->GetValue());
	} else if(widget == touchY1Ctrl) {
		inputInstance->triggerExtraValue(ExtraValues::TOUCH_Y_1, (int32_t)widget->GetValue());
	} else if(widget == touchX2Ctrl) {
		inputInstance->triggerExtraValue(ExtraValues::TOUCH_X_2, (int32_t)widget->GetValue());
	} else if(widget == touchY2Ctrl) {
		inputInstance->triggerExtraValue(ExtraValues::TOUCH_Y_2, (int32_t)widget->GetValue());
	}
}

void ExtraInputMethods::numOfTouchesChanged(wxSpinEvent& event) {
	inputInstance->triggerNumberOfTouches((uint8_t)numberOfTouchesCtrl->GetValue());
}

void ExtraInputMethods::mouseValueChanged(wxSpinEvent& event) {
	wxSpinCtrl* widget = (wxSpinCtrl*)event.GetEventObject();
	if(widget == mouseXCtrl) {
		inputInstance->triggerExtraValue(ExtraValues::MOUSE_X, (int32_t)widget->GetValue());
	} else if(widget == mouseYCtrl) {
		inputInstance->triggerExtraValue(ExtraValues::MOUSE_Y, (int32_t)widget->GetValue());
	} else if(widget == mouseVelocityXCtrl) {
		inputInstance->triggerExtraValue(ExtraValues::MOUSE_VELOCITY_X, (int32_t)widget->GetValue());
	} else if(widget == mouseVelocityYCtrl) {
		inputInstance->triggerExtraValue(ExtraValues::MOUSE_VELOCITY_Y, (int32_t)widget->GetValue());
	} else if(widget == scrollVelocityXCtrl) {
		inputInstance->triggerExtraValue(ExtraValues::SCROLL_VELOCITY_X, (int32_t)widget->GetValue());
	} else if(widget == scrollVelocityYCtrl) {
		inputInstance->triggerExtraValue(ExtraValues::SCROLL_VELOCITY_Y, (int32_t)widget->GetValue());
	}
}

void ExtraInputMethods::keyboardKeysChanged(wxListEvent& event) {
	int index                = event.GetIndex();
	nn::hid::KeyboardKey key = keyboardKeyIndices[index];
	inputInstance->triggerKeyboardButton(key);
}

void ExtraInputMethods::keyboardModifiersChanged(wxListEvent& event) {
	int index                          = event.GetIndex();
	nn::hid::KeyboardModifier modifier = keyboardModifierIndices[index];
	inputInstance->triggerKeyboardModifier(modifier);
}

void ExtraInputMethods::mouseButtonsChanged(wxListEvent& event) {
	int index                   = event.GetIndex();
	nn::hid::MouseButton button = mouseButtonIndices[index];
	inputInstance->triggerMouseButton(button);
}

void ExtraInputMethods::onClose(wxCloseEvent& event) {
	Show(false);
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
	extraInputMethodsWindow = new ExtraInputMethods(parentFrame, inputInstance, buttonData);
	extraInputMethodsWindow->Hide();

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

	extraInputMethodsWindow->updateAllValues();

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
					joyButtonToSwitch[index] = stringToButtonExtended[inputParts[0]] + (uint8_t)Btn::BUTTONS_SIZE;
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
					povToSwitch[index] = stringToButtonExtended[inputParts[0]] + (uint8_t)Btn::BUTTONS_SIZE;
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

		int pov = currentJoy->GetPOVPosition();
		if(pov != -1) {
			int povValue = pov / 9000;
			if(povToSwitch.count(povValue)) {
				int switchID = povToSwitch[povValue];
				if(povLastState != povValue) {
					if(switchID < (uint8_t)Btn::BUTTONS_SIZE) {
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