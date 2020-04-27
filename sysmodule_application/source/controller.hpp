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

	Event* vsyncEvent;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	ScreenshotHandler screenshotHandler;

	Handle applicationDebug;
	u64 applicationPID;

	void waitForVsync() {
		rc = eventWait(vsyncEvent, UINT64_MAX);
		if(R_FAILED(rc))
			fatalThrow(rc);
	}

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

	uint8_t isPaused = false;

public:
	ControllerHandler(Event* vsync, std::shared_ptr<CommunicateWithNetwork> networkImp);

	void runFrameWithPause(ControllerData controllerData);

	void setApplicationProcessId(u64 pid);

	void pauseApp() {
		if(!isPaused) {
			// Debug application again
			rc       = svcDebugActiveProcess(&applicationDebug, applicationPID);
			isPaused = true;
			std::string dhashForThisFrame;
			std::vector<uint8_t> jpegBufferForThisFrame;
			screenshotHandler.writeFramebuffer(&dhashForThisFrame, &jpegBufferForThisFrame);
			// Send these to the PC
			ADD_TO_QUEUE(RecieveGameFramebuffer, networkInstance, {
				data.buf   = jpegBufferForThisFrame;
				data.dHash = dhashForThisFrame;
			})
		}
	}

	Handle getApplicationDebugHandle() {
		return applicationDebug;
	}

	void unpauseApp() {
		if(isPaused) {
			// Unpause application
			svcCloseHandle(applicationDebug);
			isPaused = false;
		}
	}

	void reset() {
		// For now, just this
		unpauseApp();
	}

	~ControllerHandler();
};