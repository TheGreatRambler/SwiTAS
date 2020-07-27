#pragma once

#include <memory>

#ifdef __SWITCH__
#include <plog/Log.h>
#include <switch.h>
#endif

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "buttonData.hpp"
#include "screenshotHandler.hpp"

class ControllerHandler {
	// Create one for each controller index
private:
#ifdef __SWITCH__
	u64 HdlsHandle              = 0;
	HiddbgHdlsDeviceInfo device = { 0 };
	HiddbgHdlsState state       = { 0 };

	Result rc;
#endif

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

public:
	ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp);

	void setFrame(ControllerData& controllerData);
#ifdef __SWITCH__
	void setFrame(u64 buttons, JoystickPosition& left, JoystickPosition& right);
#endif

	void clearState() {
#ifdef __SWITCH__
		state.buttons                      = 0;
		state.joysticks[JOYSTICK_LEFT].dx  = 0;
		state.joysticks[JOYSTICK_LEFT].dy  = 0;
		state.joysticks[JOYSTICK_RIGHT].dx = 0;
		state.joysticks[JOYSTICK_RIGHT].dy = 0;
#endif
	}

	void setInput() {
#ifdef __SWITCH__
		rc = hiddbgSetHdlsState(HdlsHandle, &state);
		if(R_FAILED(rc)) {
			fatalThrow(rc);
		}
#endif
	}

	HiddbgHdlsState& getInput() {
		return state;
	}

	std::shared_ptr<ControllerData> getControllerData();

	~ControllerHandler();
};