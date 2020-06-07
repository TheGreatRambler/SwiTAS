#pragma once

#include "include/sol.hpp"
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

class LuaScripting {
private:
	sol::state luaState;

public:
	LuaScripting();
};