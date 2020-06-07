#include "luaScripting.hpp"

LuaScripting::LuaScripting() {
	// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html
	luaState.open_libraries(sol::lib::base);
}