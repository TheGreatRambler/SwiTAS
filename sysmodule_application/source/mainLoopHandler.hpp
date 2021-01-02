#pragma once

// clang-format off
#define ACCESS_MEMORY_SAFE(body) \ 
	if(!isPaused) { \
		rc = svcDebugActiveProcess(&applicationDebug, applicationProcessId); \
		if(R_FAILED(rc)) \
			fatalThrow(rc); \
	} \
	body \
	if(!isPaused) { \
		rc = svcCloseHandle(applicationDebug); \
		if(R_FAILED(rc)) \
			fatalThrow(rc); \
	}
// clang-format on

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <plog/Log.h>

#ifdef __SWITCH__
#include <arpa/inet.h>
#include <switch.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef YUZU
#include "yuzuSyscalls.hpp"
#endif

#include "../saltynx_plugin/source/sdkTypes.hpp"
#include "PointerChainParser.hpp"
#include "controller.hpp"
#include "gui.hpp"
#include "helpers.hpp"
#include "scripting/luaScripting.hpp"
#include "shared_network_code/networkInterface.hpp"
#include "shared_network_code/serializeUnserializeData.hpp"

struct MemoryRegionInfo {
	// mu::Parser func;
	MemoryRegionTypes type;
	uint8_t u;
	uint64_t size;
	std::string pointerDefinition;
};

class MainLoop {
private:
	uint64_t applicationProcessId = 0;
	uint64_t applicationProgramId = 0;
	std::string gameName;
	uint8_t applicationOpened = false;
	uint8_t internetConnected = false;
	uint8_t isInTASMode       = false;

	std::shared_ptr<Gui> gui;
	// Currently broken
	uint8_t printDebugInfo         = false;
	uint8_t printControllerOverlay = false;

	uint64_t heapBase;
	uint64_t mainBase;

	uint8_t lastLastController = 0;

#ifdef __SWITCH__
	HidControllerType lastControllerType;
#endif

#ifdef YUZU
	PluginDefinitions::ControllerType lastControllerType;
#endif

	std::unordered_map<std::string, FILE*> runFinalTasFileHandles;

	SerializeProtocol serializeProtocol;

#ifdef __SWITCH__
	Result rc;

	ViDisplay disp;

	Handle applicationDebug;

	PscPmModule sleepModule;

	const char* saltyPluginPath = "/SaltySD/SwiTAS_SaltyPlugin_Offsets.hex";

	uint64_t saltynxframeHasPassed                 = 0;
	uint64_t saltynxlogStringIndex                 = 0;
	uint64_t saltynxlogString                      = 0;
	uint64_t saltynxcontrollerToRecord             = 0;
	uint64_t saltynxsixAxisStateLeftJoycon         = 0;
	uint64_t saltynxsixAxisStateRightJoycon        = 0;
	uint64_t saltynxsixAxisStateLeftJoyconBacklog  = 0;
	uint64_t saltynxsixAxisStateRightJoyconBacklog = 0;
	uint64_t saltynxRecordScreenOrKeyboard         = 0;
	uint64_t saltynxtouchscreenState               = 0;
	uint64_t saltynxtouchScreenStateBacklog        = 0;
	uint64_t saltynxkeyboardState                  = 0;
	uint64_t saltynxkeyboardStateBacklog           = 0;
	uint64_t saltynxmouseState                     = 0;
	uint64_t saltynxmouseStateBacklog              = 0;
	uint64_t saltynxnumberOfTASControllers         = 0;
#endif

	uint64_t lastNanoseconds = 0;
	int lastFrameAttempt     = 0;

#ifdef __SWITCH__
	Event vsyncEvent;
	Event dmntEvent;
#endif

	std::vector<std::unique_ptr<ControllerHandler>> controllers;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	ScreenshotHandler screenshotHandler;
	std::shared_ptr<LuaScripting> luaScripting;

	// int memoryRegionCompiler;
	std::vector<MemoryRegionInfo> currentMemoryRegions;
	uint64_t mainLocation;

	uint8_t isPaused = false;

#ifdef __SWITCH__
	static char* getAppName(u64 application_id);
#endif

	void handleNetworkUpdates();
	void sendGameInfo();

	void updateGui();

	std::vector<uint8_t> getMemory(uint64_t addr, uint64_t size) {
		std::vector<uint8_t> region(size);
#ifdef __SWITCH__
		ACCESS_MEMORY_SAFE({
			svcReadDebugProcessMemory(
				region.data(), applicationDebug, addr, size);
		})
#endif
#ifdef YUZU
		yuzu_memory_readbyterange(yuzuInstance, addr, region.data(), size);
#endif
		return region;
	}

	void setMemory(uint64_t addr, std::vector<uint8_t> item) {
#ifdef __SWITCH__
		ACCESS_MEMORY_SAFE({
			svcWriteDebugProcessMemory(
				applicationDebug, item.data(), addr, item.size());
		})
#endif
#ifdef YUZU
		yuzu_memory_writebyterange(
			yuzuInstance, addr, item.data(), item.size());
#endif
	}

	template <typename T> T getMemoryType(uint64_t addr) {
		T item;
#ifdef __SWITCH__
		ACCESS_MEMORY_SAFE({
			svcReadDebugProcessMemory(&item, applicationDebug, addr, sizeof(T));
		})
#endif
#ifdef YUZU
		yuzu_rom_readbytes(yuzuInstance, addr, &item, sizeof(T));
#endif
		return item;
	}

	template <typename T> void setMemoryType(uint64_t addr, T item) {
#ifdef __SWITCH__
		ACCESS_MEMORY_SAFE({
			svcWriteDebugProcessMemory(
				applicationDebug, &item, addr, sizeof(T));
		})
#endif
#ifdef YUZU
		yuzu_rom_writebytes(yuzuInstance, addr, &item, sizeof(T));
#endif
	}

	template <typename T>
	std::string memoryToString(std::vector<uint8_t>& bytes) {
		return std::to_string(*(T*)bytes.data());
	}

	void pauseApp(uint8_t linkedWithFrameAdvance, uint8_t includeFramebuffer,
		TasValueToRecord typeTospoof, uint32_t frame, uint16_t savestateHookNum,
		uint32_t branchIndex, uint8_t playerIndex);

	void waitForVsync();

	void unpauseApp() {
		if(isPaused) {
#ifdef __SWITCH__
			// Unpause application
			rc = svcCloseHandle(applicationDebug);
			if(R_FAILED(rc))
				fatalThrow(rc);
			isPaused = false;
#endif
#ifdef YUZU
			yuzu_emu_unpause(yuzuInstance);
#endif
		}
	}

	// This assumes that the app is paused
	void runSingleFrame(uint8_t linkedWithFrameAdvance,
		uint8_t includeFramebuffer, TasValueToRecord typeTospoof,
		uint32_t frame, uint16_t savestateHookNum, uint32_t branchIndex,
		uint8_t playerIndex);

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
	uint8_t getLastController() {
		return controllers.size();
	}

	void setSixAxisRecord(int32_t controller);
	void setSixAxisSpoof();
	void recordAllSixAxis();
	void spoofAllKeyboardTouch();
	void recordAllKeyboardTouch();
	void setKeyboardRecord();
	void setTouchRecord();
	void getSixAxisState(int32_t controller, ControllerData* state);
	void setSixAxisState(int32_t controller, ControllerData* state);
	void getTouchState(TouchAndKeyboardData* state);
	void setTouchState(TouchAndKeyboardData* state);
	void getKeyboardMouseState(TouchAndKeyboardData* state);
	void setKeyboardMouseState(TouchAndKeyboardData* state);
	void setDockedMode();
	void setHandheldMode();

	void spoofAll() {
		setSixAxisSpoof();
		spoofAllKeyboardTouch();
	}

	void recordAll() {
		recordAllSixAxis();
		recordAllKeyboardTouch();
	}

	void clearExtraData() {
		TouchAndKeyboardData empty;
		setTouchState(&empty);
		setKeyboardMouseState(&empty);
	}

	void setAll() { }

	uint8_t finalTasShouldRun;
	void runFinalTas(
		std::vector<std::string> scriptPaths, std::string extraDataPath);

	uint8_t checkSleep();
	uint8_t checkAwaken();

public:
	MainLoop();

	void mainLoopHandler();

	~MainLoop();
};