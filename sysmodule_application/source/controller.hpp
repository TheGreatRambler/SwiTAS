#pragma once

#include <memory>
#include <plog/Log.h>
#include <switch.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "buttonData.hpp"
#include "screenshotHandler.hpp"

class ControllerHandler {
	// Create one for each controller index
private:
	u64 HdlsHandle              = 0;
	HiddbgHdlsDeviceInfo device = { 0 };
	HiddbgHdlsState state       = { 0 };

	Result rc;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

public:
	ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp);

	void setFrame(ControllerData controllerData);
	void setFrame(u64 buttons, JoystickPosition& left, JoystickPosition& right);

	void clearState() {
		state.buttons                      = 0;
		state.joysticks[JOYSTICK_LEFT].dx  = 0;
		state.joysticks[JOYSTICK_LEFT].dy  = 0;
		state.joysticks[JOYSTICK_RIGHT].dx = 0;
		state.joysticks[JOYSTICK_RIGHT].dy = 0;
	}

	void setInput() {
		rc = hiddbgSetHdlsState(HdlsHandle, &state);
		if(R_FAILED(rc)) {
			fatalThrow(rc);
		}
	}

	std::shared_ptr<ControllerData> getControllerData();

	~ControllerHandler();
};