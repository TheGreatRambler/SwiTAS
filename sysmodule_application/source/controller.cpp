#include "controller.hpp"

#ifdef YUZU
ControllerHandler::ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp, std::shared_ptr<Syscalls> syscalls) {
	yuzuSyscalls = syscalls;
#endif
#ifdef __SWITCH__
	ControllerHandler::ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp, HidControllerID startingID) {
#endif
		networkInstance = networkImp;

#ifdef __SWITCH__
		deviceLeftJoycon.deviceType = HidDeviceType_JoyLeft2;

		deviceLeftJoycon.npadInterfaceType = NpadInterfaceType_Bluetooth;

		deviceLeftJoycon.singleColorBody    = RGBA8_MAXALPHA(0x00, 0x00, 0xFF);
		deviceLeftJoycon.singleColorButtons = RGBA8_MAXALPHA(0xFF, 0x00, 0x00);
		deviceLeftJoycon.colorLeftGrip      = deviceLeftJoycon.singleColorBody;
		deviceLeftJoycon.colorRightGrip     = deviceLeftJoycon.singleColorBody;

		stateLeftJoycon.batteryCharge = 4;

		stateLeftJoycon.buttons                      = 0;
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx  = 0;
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy  = 0;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dx = 0;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dy = 0;

		rc = hiddbgAttachHdlsVirtualDevice(&hdlsHandleLeftJoycon, &deviceLeftJoycon);
		if(R_FAILED(rc))
			fatalThrow(rc);

		rc = hiddbgSetHdlsState(hdlsHandleLeftJoycon, &stateLeftJoycon);
		if(R_FAILED(rc))
			fatalThrow(rc);

		deviceRightJoycon.deviceType = HidDeviceType_JoyRight1;

		deviceRightJoycon.npadInterfaceType = NpadInterfaceType_Bluetooth;

		deviceRightJoycon.singleColorBody    = RGBA8_MAXALPHA(0x00, 0x00, 0xFF);
		deviceRightJoycon.singleColorButtons = RGBA8_MAXALPHA(0xFF, 0x00, 0x00);
		deviceRightJoycon.colorLeftGrip      = deviceRightJoycon.singleColorBody;
		deviceRightJoycon.colorRightGrip     = deviceRightJoycon.singleColorBody;

		stateRightJoycon.batteryCharge = 4;

		stateRightJoycon.buttons                      = 0;
		stateRightJoycon.joysticks[JOYSTICK_LEFT].dx  = 0;
		stateRightJoycon.joysticks[JOYSTICK_LEFT].dy  = 0;
		stateRightJoycon.joysticks[JOYSTICK_RIGHT].dx = 0;
		stateRightJoycon.joysticks[JOYSTICK_RIGHT].dy = 0;

		rc = hiddbgAttachHdlsVirtualDevice(&hdlsHandleRightJoycon, &deviceRightJoycon);
		if(R_FAILED(rc))
			fatalThrow(rc);

		rc = hiddbgSetHdlsState(hdlsHandleRightJoycon, &stateRightJoycon);
		if(R_FAILED(rc))
			fatalThrow(rc);

#endif

#ifdef YUZU
		yuzuSyscalls->function_joypad_addjoypad(yuzuSyscalls->getYuzuInstance());
#endif
	}

	void ControllerHandler::setFrame(ControllerData & controllerData) {
		clearState();
// Set data one at a time
#ifdef __SWITCH__
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx  = controllerData.LS_X;
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy  = controllerData.LS_Y;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dx = controllerData.RS_X;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dy = controllerData.RS_Y;
		for(auto const& button : btnToHidKeys) {
			if(GET_BIT(controllerData.buttons, (uint8_t)button.first)) {
				stateLeftJoycon.buttons |= button.second;
			}
		}
#endif

		setInput();
	}

	void ControllerHandler::setFrame(u64 buttons, int32_t leftX, int32_t leftY, int32_t rightX, int32_t rightY) {
		clearState();

#ifdef __SWITCH__
		stateLeftJoycon.buttons                      = buttons;
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx  = leftX;
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy  = leftY;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dx = rightX;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dy = rightY;
#endif

		setInput();
	}

	std::shared_ptr<ControllerData> ControllerHandler::getControllerData() {
		std::shared_ptr<ControllerData> newControllerData = std::make_shared<ControllerData>();

		for(auto const& button : btnToHidKeys) {
#ifdef __SWITCH__
			if(stateLeftJoycon.buttons & button.second) {
				SET_BIT(newControllerData->buttons, true, (uint8_t)button.first);
			} else {
				SET_BIT(newControllerData->buttons, false, (uint8_t)button.first);
			}
#endif
		}

#ifdef __SWITCH__
		newControllerData->LS_X = stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx;
		newControllerData->LS_Y = stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy;
		newControllerData->RS_X = stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dx;
		newControllerData->RS_Y = stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dy;
#endif

		// Accel TODO

		newControllerData->frameState = 0;

		return newControllerData;
	}

	ControllerHandler::~ControllerHandler() {
// Detatch Controller
#ifdef __SWITCH__
		rc = hiddbgDetachHdlsVirtualDevice(hdlsHandleLeftJoycon);
		if(R_FAILED(rc))
			fatalThrow(rc);

		rc = hiddbgDetachHdlsVirtualDevice(hdlsHandleRightJoycon);
		if(R_FAILED(rc))
			fatalThrow(rc);
#endif
	}