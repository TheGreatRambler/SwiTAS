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
	uint8_t isInTASMode       = false;
	Handle applicationDebug;

	uint8_t autoRunOn = false;
	u64 nanosecondsBetweenAutorun;
	u64 lastAutorunTime = 0;

	Event vsyncEvent;

	// WILL BE MORE
	std::vector<std::unique_ptr<ControllerHandler>> controllers;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	std::vector<std::pair<uint64_t, uint64_t>> memoryRegions;

	ScreenshotHandler screenshotHandler;

	uint8_t isPaused = false;

	static char* getAppName(u64 application_id);

	void handleNetworkUpdates();
	void sendGameInfo();

	GameMemoryInfo getGameMemoryInfo(MemoryInfo memInfo);

	void pauseApp();

	void waitForVsync() {
		rc = eventWait(&vsyncEvent, UINT64_MAX);
		if(R_FAILED(rc))
			fatalThrow(rc);
	}

	void unpauseApp() {
		if(isPaused) {
			// Unpause application
			svcCloseHandle(applicationDebug);
			isPaused = false;
		}
	}

	// This assumes that the app is paused
	void runSingleFrame();

	void reset() {
		// For now, just this
		unpauseApp();
	}

	// This allows you to use the inputs in a real controller
	// to match a TAS controller, so you don't get stuck while in TAS mode
	void matchFirstControllerToTASController(uint8_t player);

	// Deletes all controllers upon being started, hid:dbg as well as normal
	// controllers. Otherwise, it sets the number of hid:dbg controllers
	void setControllerNumber(uint8_t numOfControllers);
	uint8_t getNumControllers() {
		uint8_t num = 0;
		for(uint8_t i = 0; i < 8; i++) {
			// Handheld controller is separate
			if(!hidIsControllerConnected((HidControllerID)i)) {
				num = i;
			}
		}
		if(hidIsControllerConnected(CONTROLLER_HANDHELD)) {
			num++;
		}
		return num;
	}

public:
	MainLoop();

	void mainLoopHandler();

	~MainLoop();
};