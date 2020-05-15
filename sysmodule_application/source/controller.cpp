#include "controller.hpp"

ControllerHandler::ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp) {
	networkInstance = networkImp;

	// Types include:
	// - HidDeviceType_FullKey3
	// - HidDeviceType_JoyLeft2
	// - HidDeviceType_JoyRight1
	// - HidDeviceType_LarkLeftHVC
	// - HidDeviceType_LarkRightHVC
	// - HidDeviceType_LarkLeftNES
	// - HidDeviceType_System19
	device.deviceType = HidDeviceType_FullKey3;

	// Set the interface type
	device.npadInterfaceType = NpadInterfaceType_Bluetooth;

	// Colors
	// Colors hardcoded here
	device.singleColorBody    = RGBA8_MAXALPHA(0x00, 0x00, 0xFF);
	device.singleColorButtons = RGBA8_MAXALPHA(0xFF, 0x00, 0x00);
	device.colorLeftGrip      = device.singleColorBody;
	device.colorRightGrip     = device.singleColorBody;

	// Charge is max
	state.batteryCharge = 4;

	// Set Buttons and Joysticks
	clearState();

	// Attach the controller
	rc = hiddbgAttachHdlsVirtualDevice(&HdlsHandle, &device);
	if(R_FAILED(rc))
		fatalThrow(rc);

	// Update the state with the zero initialized struct
	setInput();
}

void ControllerHandler::setFrame(ControllerData controllerData) {
	clearState();
	// Set data one at a time
	state.joysticks[JOYSTICK_LEFT].dx  = controllerData.LS_X;
	state.joysticks[JOYSTICK_LEFT].dy  = controllerData.LS_Y;
	state.joysticks[JOYSTICK_RIGHT].dx = controllerData.RS_X;
	state.joysticks[JOYSTICK_RIGHT].dy = controllerData.RS_Y;
	for(auto const& button : btnToHidKeys) {
		if(GET_BIT(controllerData.buttons, button.first)) {
			state.buttons |= button.second;
		}
	}

	setInput();
}

void ControllerHandler::setFrame(u64 buttons, JoystickPosition& left, JoystickPosition& right) {
	clearState();

	state.buttons                      = buttons;
	state.joysticks[JOYSTICK_LEFT].dx  = left.dx;
	state.joysticks[JOYSTICK_LEFT].dy  = left.dy;
	state.joysticks[JOYSTICK_RIGHT].dx = right.dx;
	state.joysticks[JOYSTICK_RIGHT].dy = right.dy;

	setInput();
}

std::shared_ptr<ControllerData> ControllerHandler::getControllerData() {
	std::shared_ptr<ControllerData> newControllerData = std::make_shared<ControllerData>();

	for(auto const& button : btnToHidKeys) {
		if(state.buttons & button.second) {
			SET_BIT(newControllerData->buttons, true, button.first);
		} else {
			SET_BIT(newControllerData->buttons, false, button.first);
		}
	}

	newControllerData->LS_X = state.joysticks[JOYSTICK_LEFT].dx;
	newControllerData->LS_Y = state.joysticks[JOYSTICK_LEFT].dy;
	newControllerData->RS_X = state.joysticks[JOYSTICK_RIGHT].dx;
	newControllerData->RS_Y = state.joysticks[JOYSTICK_RIGHT].dy;

	// Accel TODO

	newControllerData->frameState = 0;

	return newControllerData;
}

ControllerHandler::~ControllerHandler() {
	// Detatch Controller
	rc = hiddbgDetachHdlsVirtualDevice(HdlsHandle);
	if(R_FAILED(rc))
		fatalThrow(rc);
}