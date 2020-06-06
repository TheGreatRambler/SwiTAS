#include "luaScripting.hpp"

LuaScripting::LuaScripting() {
	luaState.open_libraries(sol::lib::base);
}