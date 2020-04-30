#include "controller.hpp"

ControllerHandler::ControllerHandler(Event* vsync, std::shared_ptr<CommunicateWithNetwork> networkImp) {
	vsyncEvent      = vsync;
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
	state.buttons                      = 0;
	state.joysticks[JOYSTICK_LEFT].dx  = 0;
	state.joysticks[JOYSTICK_LEFT].dy  = 0;
	state.joysticks[JOYSTICK_RIGHT].dx = 0;
	state.joysticks[JOYSTICK_RIGHT].dy = 0;

	// Attach the controller
	rc = hiddbgAttachHdlsVirtualDevice(&HdlsHandle, &device);
	if(R_FAILED(rc))
		fatalThrow(rc);

	// Update the state with the zero initialized struct
	setInput();
}

void ControllerHandler::runFrameWithPause(ControllerData controllerData) {
	// Set data one at a time
	state.joysticks[JOYSTICK_LEFT].dx  = controllerData.LS_X;
	state.joysticks[JOYSTICK_LEFT].dy  = controllerData.LS_Y;
	state.joysticks[JOYSTICK_RIGHT].dx = controllerData.RS_X;
	state.joysticks[JOYSTICK_RIGHT].dy = controllerData.RS_Y;

	state.buttons = 0;
	for(auto const& button : btnToHidKeys) {
		if(GET_BIT(controllerData.buttons, button.first)) {
			state.buttons |= button.second;
		}
	}

	setInput();

	unpauseApp();
	waitForVsync();
	pauseApp();

	screenshotHandler.writeFramebuffer(networkInstance);
}

void ControllerHandler::setApplicationProcessId(u64 pid) {
	applicationPID = pid;
	// Instantly pause the app
	LOGD << "Start pausing app";
	// TODO implement real stuff
	waitForVsync();
	pauseApp();
}

ControllerHandler::~ControllerHandler() {
	// Detatch Controller
	rc = hiddbgDetachHdlsVirtualDevice(HdlsHandle);
	if(R_FAILED(rc))
		fatalThrow(rc);
}