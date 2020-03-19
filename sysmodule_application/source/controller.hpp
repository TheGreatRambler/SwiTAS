#pragma once

#include <switch.h>

#include "buttonData.hpp"
#include "screenshotHandler.hpp"

class ControllerHandler {
	// Create one for each controller index
private:
	u64 HdlsHandle              = 0;
	HiddbgHdlsDeviceInfo device = { 0 };
	HiddbgHdlsState state       = { 0 };

	Result rc;

	Event vsyncEvent;

	ScreenshotHandler screenshotHandler;

	void waitForVsync() {
		rc = eventWait(&vsyncEvent, U64_MAX);
		if(R_FAILED(rc))
			fatalThrow(rc);
	}

	void setInput() {
		rc = hiddbgSetHdlsState(HdlsHandle, &state);
		if(R_FAILED(rc))
			fatalThrow(rc);
	}

public:
	ControllerHandler();

	void runFrameWithPause(ControllerData controllerData);

	~ControllerHandler();
};