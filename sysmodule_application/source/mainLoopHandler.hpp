#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef __SWITCH__
#include "dmntcht/dmntcht.hpp"
#include <plog/Log.h>
#include <switch.h>
#endif

#ifdef YUZU
#include "yuzuSyscalls.hpp"
#endif

#include "PointerChainParser.hpp"
#include "controller.hpp"
#include "gui.hpp"
#include "scripting/luaScripting.hpp"
#include "sharedNetworkCode/networkInterface.hpp"
#include "sharedNetworkCode/serializeUnserializeData.hpp"

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

	std::unordered_map<std::string, FILE*> runFinalTasFileHandles;

	SerializeProtocol serializeProtocol;

#ifdef __SWITCH__
	Result rc;

	ViDisplay disp;

	PscPmModule sleepModule;
	Waiter sleepModeWaiter;

	uint64_t lastNanoseconds = 0;
	int lastFrameAttempt     = 0;

	uint64_t saltynxFrameHasPassed;
	uint64_t saltynxLogStringIndex;
	uint64_t saltynxLogString;
	uint64_t saltynxSixAxisStateLeftJoycon;
	uint64_t saltynxSixAxisStateRightJoycon;
	uint64_t saltynxTouchscreenState;
	uint64_t saltynxSixAxisStateLeftJoyconBacklog;
	uint64_t saltynxSixAxisStateRightJoyconBacklog;
	uint64_t saltynxOriginalTouchscreenState;
#endif

#ifdef YUZU
	std::shared_ptr<Syscalls> yuzuSyscalls;
#endif

#ifdef __SWITCH__
	Event vsyncEvent;
#endif

	std::vector<std::unique_ptr<ControllerHandler>> controllers;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	ScreenshotHandler screenshotHandler;
	std::shared_ptr<LuaScripting> luaScripting;

	// int memoryRegionCompiler;
	std::vector<MemoryRegionInfo> currentMemoryRegions;
	uint64_t mainLocation;

	uint8_t isPaused = false;

	void readFullFileData(FILE* file, void* bufPtr, int size) {
		int sizeActuallyRead = 0;
		uint8_t* buf         = (uint8_t*)bufPtr;

		while(sizeActuallyRead != size) {
			int bytesRead = fread(&buf[sizeActuallyRead], size - sizeActuallyRead, 1, file);
			sizeActuallyRead += bytesRead;
		}
	}

	void replaceInString(std::string input, std::string initial, std::string final) {
		int index;
		while((index = input.find(initial)) != std::string::npos) {
			input.replace(index, final.length(), final);
		}
	}

#ifdef __SWITCH__
	static char* getAppName(u64 application_id);
#endif

	// https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}

	void handleNetworkUpdates();
	void sendGameInfo();

	void updateGui();

	// void prepareMemoryRegionMath(mu::Parser& parser, std::string func);

	std::vector<uint8_t> getMemory(uint64_t addr, uint64_t size) {
		std::vector<uint8_t> region(size);
#ifdef __SWITCH__
		dmntchtReadCheatProcessMemory(addr, region.data(), size);
#endif
#ifdef YUZU
		yuzuSyscalls->function_rom_readbytes(yuzuSyscalls->getYuzuInstance(), region.data(), addr, size);
#endif
		return region;
	}

	template <typename T> std::string memoryToString(std::vector<uint8_t>& bytes) {
		return std::to_string(*(T*)bytes.data());
	}

	std::string getJsonElement(uint8_t tabs, std::string key, std::string value) {
		return std::string(tabs, '\t') + "\"" + key + "\": \"" + value + "\",\n";
	}

	std::string getJsonElementNum(uint8_t tabs, std::string key, std::string value) {
		return std::string(tabs, '\t') + "\"" + key + "\": " + value + ",\n";
	}

	void pauseApp(uint8_t linkedWithFrameAdvance, uint8_t includeFramebuffer, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint32_t branchIndex, uint8_t playerIndex);

	void waitForVsync();

	void unpauseApp() {
		if(isPaused) {
#ifdef __SWITCH__
			// Unpause application
			dmntchtResumeCheatProcess();
			isPaused = false;
#endif
#ifdef YUZU
			yuzuSyscalls->function_emu_unpause(yuzuSyscalls->getYuzuInstance());
#endif
		}
	}

	// This assumes that the app is paused
	void runSingleFrame(uint8_t linkedWithFrameAdvance, uint8_t includeFramebuffer, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint32_t branchIndex, uint8_t playerIndex);

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

	uint8_t finalTasShouldRun;
	void runFinalTas(std::vector<std::string> scriptPaths);

	uint8_t checkSleep();
	uint8_t checkAwaken();

public:
	MainLoop();

#ifdef YUZU
	std::shared_ptr<Syscalls> getYuzuSyscalls() {
		return yuzuSyscalls;
	}
#endif

	void mainLoopHandler();

	~MainLoop();
};