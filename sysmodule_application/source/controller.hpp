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
#include "sharedNetworkCode/networkInterface.hpp"

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

#ifdef YUZU
	std::shared_ptr<Syscalls> yuzuSyscalls;
#endif

public:
#ifdef YUZU
	ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp, std::shared_ptr<Syscalls> syscalls);
#endif
#ifdef __SWITCH__
	ControllerHandler(std::shared_ptr<CommunicateWithNetwork> networkImp);
#endif

	void setFrame(ControllerData& controllerData);
	void setFrame(u64 buttons, int32_t leftX, int32_t leftY, int32_t rightX, int32_t rightY);

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