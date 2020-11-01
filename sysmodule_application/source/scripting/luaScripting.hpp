#pragma once

#include <memory>
#include <sol/sol.hpp>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef YUZU
#include "../yuzuSyscalls.hpp"
#endif

class LuaScripting {
private:
	sol::state luaState;
	uint8_t isClosing = false;

	std::string luaPath;

	void registerLuaFuncs();

public:
	LuaScripting();
	// Called on init
	void loadScript(std::string path);

	// Called prior to destruction
	void endScript();

	// Called on app init
	void callMainloop();

	// Called on a SwiTAS frame advance
	void onFrameAdvance(uint8_t isRunFinalTas);

	void setClosing() {
		isClosing = true;
	}
};