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

	Event* vsyncEvent;

	ScreenshotHandler screenshotHandler;

	Handle applicationDebug;
	u64 applicationPID;

	void waitForVsync() {
		rc = eventWait(vsyncEvent, UINT64_MAX);
		if(R_FAILED(rc))
			fatalThrow(rc);
	}

	void setInput() {
		rc = hiddbgSetHdlsState(HdlsHandle, &state);
		if(R_FAILED(rc))
			fatalThrow(rc);
	}

	void pauseApp() {
		if(!isPaused) {
			// Debug application again
			rc       = svcDebugActiveProcess(&applicationDebug, applicationPID);
			isPaused = true;
		}
	}

	void unpauseApp() {
		if(isPaused) {
			// Unpause application
			svcCloseHandle(applicationDebug);
			isPaused = false;
		}
	}

	uint8_t isPaused = false;

public:
	ControllerHandler(Event* vsync);

	void runFrameWithPause(ControllerData controllerData);

	void setApplicationProcessId(u64 pid);

	void reset() {
		// For now, just this
		unpauseApp();
	}

	~ControllerHandler();
};