#pragma once

#include <memory>

#ifdef __SWITCH__
#include <plog/Log.h>
#include <switch.h>
#endif

#ifdef YUZU
#include "yuzuSyscalls.hpp"
#endif

#include "buttonData.hpp"
#include "screenshotHandler.hpp"
#include "shared_network_code/networkInterface.hpp"

class ControllerHandler {
	// Create one for each controller index
private:
#ifdef __SWITCH__
	u64 hdlsHandleLeftJoycon              = 0;
	HiddbgHdlsDeviceInfo deviceLeftJoycon = { 0 };
	HiddbgHdlsState stateLeftJoycon       = { 0 };

	u64 hdlsHandleRightJoycon              = 0;
	HiddbgHdlsDeviceInfo deviceRightJoycon = { 0 };
	HiddbgHdlsState stateRightJoycon       = { 0 };

	Result rc;
#endif

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

#ifdef YUZU
	std::shared_ptr<Syscalls> yuzuSyscalls;
#endif

public:
#ifdef YUZU
	ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp, std::shared_ptr<Syscalls> syscalls);
#endif
#ifdef __SWITCH__
	ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp, HidControllerID startingID);
#endif

	void setFrame(ControllerData& controllerData);
	void setFrame(u64 buttons, int32_t leftX, int32_t leftY, int32_t rightX, int32_t rightY);

	void clearState() {
#ifdef __SWITCH__
		stateLeftJoycon.buttons                      = 0;
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx  = 0;
		stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy  = 0;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dx = 0;
		stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dy = 0;
#endif
	}

	void setInput() {
#ifdef __SWITCH__
		rc = hiddbgSetHdlsState(hdlsHandleLeftJoycon, &stateLeftJoycon);
		if(R_FAILED(rc)) {
			fatalThrow(rc);
		}
#endif
	}

	HiddbgHdlsState& getInput() {
		return stateLeftJoycon;
	}

	std::shared_ptr<ControllerData> getControllerData();

	~ControllerHandler();
};