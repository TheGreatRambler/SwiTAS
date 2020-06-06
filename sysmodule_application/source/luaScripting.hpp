#pragma once

#include <sol.hpp>
#include <string>

class LuaScripting {
private:
	sol::state luaState;

public:
	LuaScripting();
}