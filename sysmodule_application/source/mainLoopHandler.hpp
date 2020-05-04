#pragma once

#include <cstring>
#include <memory>
#include <plog/Log.h>
#include <switch.h>
#include <vector>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "controller.hpp"

class MainLoop {
private:
	Result rc;
	u64 applicationProcessId = 0;
	u64 applicationProgramId = 0;
	std::string gameName;
	uint8_t applicationOpened = false;
	uint8_t internetConnected = false;

	Event vsyncEvent;

	// WILL BE MORE
	std::unique_ptr<ControllerHandler> controller;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	std::vector<std::pair<uint64_t, uint64_t>> memoryRegions;

	ScreenshotHandler screenshotHandler;

	Handle applicationDebug;
	u64 applicationPID;

	uint8_t isPaused = false;

	static char* getAppName(u64 application_id);

	void handleNetworkUpdates();
	void sendGameInfo();

	GameMemoryInfo getGameMemoryInfo(MemoryInfo memInfo);

	void pauseApp();

	void waitForVsync() {
		rc = eventWait(vsyncEvent, UINT64_MAX);
		if(R_FAILED(rc))
			fatalThrow(rc);
	}

	void getFramebuffer() {
		screenshotHandler.writeFramebuffer(networkInstance);
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

public:
	MainLoop();

	void mainLoopHandler();

	~MainLoop();
};