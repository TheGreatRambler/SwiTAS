#pragma once

#include <cstring>
#include <memory>
#include <vector>

#ifdef __SWITCH__
#include <plog/Log.h>
#include <switch.h>
#endif

#ifdef YUZU
#include "scripting/dllFunctionDefinitions.hpp"
#endif

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "controller.hpp"
#include "scripting/luaScripting.hpp"

class MainLoop {
private:
	uint64_t applicationProcessId = 0;
	uint64_t applicationProgramId = 0;
	std::string gameName;
	uint8_t applicationOpened = false;
	uint8_t internetConnected = false;
	uint8_t isInTASMode       = false;

#ifdef __SWITCH__
	Result rc;
	Handle applicationDebug;
#endif

	uint8_t autoRunOn = false;
	uint32_t autoRunFrameframe;
	uint16_t auroRunSavestateHookNum;
	uint64_t nanosecondsBetweenAutorun;
	uint64_t lastAutorunTime = 0;

#ifdef __SWITCH__
	Event vsyncEvent;
#endif

#ifdef YUZU
	void* yuzuInstance;
#endif

	std::vector<std::unique_ptr<ControllerHandler>> controllers;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;
	std::vector<std::pair<uint64_t, uint64_t>> memoryRegions;

	ScreenshotHandler screenshotHandler;
	LuaScripting luaScripting;

	uint8_t isPaused = false;

#ifdef __SWITCH__
	static char* getAppName(u64 application_id);
#endif

	void handleNetworkUpdates();
	void sendGameInfo();

#ifdef __SWITCH__
	GameMemoryInfo getGameMemoryInfo(MemoryInfo memInfo);
#endif

	void pauseApp(uint8_t linkedWithFrameAdvance, uint32_t frame, uint16_t savestateHookNum, uint8_t playerIndex);

	void waitForVsync() {
#ifdef __SWITCH__
		rc = eventWait(&vsyncEvent, UINT64_MAX);
		if(R_FAILED(rc))
			fatalThrow(rc);
#endif
	}

	void unpauseApp() {
		if(isPaused) {
#ifdef __SWITCH__
			// Unpause application
			svcCloseHandle(applicationDebug);
			isPaused = false;
#endif
		}
	}

	// This assumes that the app is paused
	void runSingleFrame(uint8_t linkedWithFrameAdvance, uint32_t frame, uint16_t savestateHookNum, uint8_t playerIndex);

	void clearEveryController();

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
	uint8_t getNumControllers();

public:
	MainLoop();

#ifdef YUZU
	void setYuzuInstance(void* instance) {
		yuzuInstance = instance;
	}
#endif

	void mainLoopHandler();

	LuaScripting& getLuaScripting();

	~MainLoop();
};