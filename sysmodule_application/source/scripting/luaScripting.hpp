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

class LuaScripting {
private:
	sol::state luaState;
	uint8_t scriptLoaded = false;
	std::unique_ptr<std::thread> luaExecutionThread;

	std::string luaPath;

	uint8_t syscallReady             = false;
	std::mutex syscallMutex;
	std::condition_variable syscallCv;

public:
	LuaScripting();

	void loadScript(std::string path);

	void endScript();

	void luaThread();

	void callMainloop();
};