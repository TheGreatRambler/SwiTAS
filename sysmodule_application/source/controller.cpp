#include "controller.hpp"

ControllerHandler::ControllerHandler() {
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
	state.buttons                      = 0;
	state.joysticks[JOYSTICK_LEFT].dx  = 0;
	state.joysticks[JOYSTICK_LEFT].dy  = 0;
	state.joysticks[JOYSTICK_RIGHT].dx = 0;
	state.joysticks[JOYSTICK_RIGHT].dy = 0;

	// Attach the controller
	rc = hiddbgAttachHdlsVirtualDevice(&HdlsHandle, &device);
	if(R_FAILED(rc))
		fatalThrow(rc);

	// Update the state
	rc = hiddbgSetHdlsState(HdlsHandle, &state);
	if(R_FAILED(rc))
		fatalThrow(rc);
}

void ControllerHandler::runFrameWithPause(ControllerData controllerData) {
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

	// Undebug application TODO <--
	waitForVsync();
	setInput();

	std::string dhashForThisFrame;
	std::vector<uint8_t> jpegBufferForThisFrame;
	screenshotHandler.writeFramebuffer(&dhashForThisFrame, &jpegBufferForThisFrame);
	// Send these to the PC

	// Debug the application again TODO
}

ControllerHandler::~ControllerHandler() {
	// Detatch Controller
	rc = hiddbgDetachHdlsVirtualDevice(HdlsHandle);
	if(R_FAILED(rc))
		fatalThrow(rc);
}