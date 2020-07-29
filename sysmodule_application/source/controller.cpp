#include "controller.hpp"

#ifdef YUZU
ControllerHandler::ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp, std::shared_ptr<Syscalls> syscalls) {
	yuzuSyscalls = syscalls;
#endif
#ifdef __SWITCH__
	ControllerHandler::ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp) {
#endif
		networkInstance = networkImp;

#ifdef __SWITCH__
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
#endif

#ifdef YUZU
		yuzuSyscalls->function_joypad_addjoypad(yuzuSyscalls->getYuzuInstance());
#endif

		// Set Buttons and Joysticks
		clearState();

#ifdef __SWITCH__
		// Attach the controller
		rc = hiddbgAttachHdlsVirtualDevice(&HdlsHandle, &device);
		if(R_FAILED(rc))
			fatalThrow(rc);
#endif

		// Update the state with the zero initialized struct
		setInput();
	}

	void ControllerHandler::setFrame(ControllerData & controllerData) {
		clearState();
// Set data one at a time
#ifdef __SWITCH__
		state.joysticks[JOYSTICK_LEFT].dx  = controllerData.LS_X;
		state.joysticks[JOYSTICK_LEFT].dy  = controllerData.LS_Y;
		state.joysticks[JOYSTICK_RIGHT].dx = controllerData.RS_X;
		state.joysticks[JOYSTICK_RIGHT].dy = controllerData.RS_Y;
		for(auto const& button : btnToHidKeys) {
			if(GET_BIT(controllerData.buttons, button.first)) {
				state.buttons |= button.second;
			}
		}
#endif

		setInput();
	}

	void ControllerHandler::setFrame(u64 buttons, int32_t leftX, int32_t leftY, int32_t rightX, int32_t rightY) {
		clearState();

#ifdef __SWITCH__
		state.buttons                      = buttons;
		state.joysticks[JOYSTICK_LEFT].dx  = leftX;
		state.joysticks[JOYSTICK_LEFT].dy  = leftY;
		state.joysticks[JOYSTICK_RIGHT].dx = rightX;
		state.joysticks[JOYSTICK_RIGHT].dy = rightY;
#endif

		setInput();
	}

	std::shared_ptr<ControllerData> ControllerHandler::getControllerData() {
		std::shared_ptr<ControllerData> newControllerData = std::make_shared<ControllerData>();

		for(auto const& button : btnToHidKeys) {
#ifdef __SWITCH__
			if(state.buttons & button.second) {
				SET_BIT(newControllerData->buttons, true, button.first);
			} else {
				SET_BIT(newControllerData->buttons, false, button.first);
			}
#endif
		}

#ifdef __SWITCH__
		newControllerData->LS_X = state.joysticks[JOYSTICK_LEFT].dx;
		newControllerData->LS_Y = state.joysticks[JOYSTICK_LEFT].dy;
		newControllerData->RS_X = state.joysticks[JOYSTICK_RIGHT].dx;
		newControllerData->RS_Y = state.joysticks[JOYSTICK_RIGHT].dy;
#endif

		// Accel TODO

		newControllerData->frameState = 0;

		return newControllerData;
	}

	ControllerHandler::~ControllerHandler() {
// Detatch Controller
#ifdef __SWITCH__
		rc = hiddbgDetachHdlsVirtualDevice(HdlsHandle);
		if(R_FAILED(rc))
			fatalThrow(rc);
#endif
	}