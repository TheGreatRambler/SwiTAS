#include "luaScripting.hpp"

LuaScripting::LuaScripting() {
	// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html
	// https://github.com/yuzu-emu/yuzu/wiki/Building-for-Windows
	luaState.open_libraries(sol::lib::base);
}