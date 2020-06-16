#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <sol/sol.hpp>
#include <string>
#include <thread>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "../yuzuSyscalls.hpp"

class LuaScripting {
private:
	sol::state luaState;
	uint8_t scriptLoaded = false;
	std::unique_ptr<std::thread> luaExecutionThread;

	std::string luaPath;

	std::atomic_bool syscallReady = false;
	uint8_t ready                 = false;
	uint8_t processed             = false;
	std::mutex syscallMutex;
	std::condition_variable syscallCv;

	void sendSyscall(std::function<void()> func);

	void luaThread();

public:
	LuaScripting();

	void loadScript(std::string path);

	void endScript();

	void callMainloop();
};