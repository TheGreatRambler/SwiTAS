#pragma once

#include <sol/sol.hpp>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

class LuaScripting {
private:
	sol::state luaState;
	uint8_t scriptLoaded = false;

public:
	LuaScripting();

	void loadScript(std::string path);

	void endScript();

	void callLuaMainloop();
};