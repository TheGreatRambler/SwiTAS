#include "luaScripting.hpp"

LuaScripting::LuaScripting() {
	// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html
	// https://github.com/yuzu-emu/yuzu/wiki/Building-for-Windows
	// https://sol2.readthedocs.io/en/latest/api/function.html
	// https://sol2.readthedocs.io/en/latest/tutorial/all-the-things.html
	luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine, sol::lib::string, sol::lib::os, sol::lib::math, sol::lib::table, sol::lib::bit32, sol::lib::io, sol::lib::utf8);
}

void LuaScripting::loadScript(std::string path) {
	sol::load_result currentScript = luaState.load_file(path);
	if(!currentScript.valid()) {
		/*
		sol::error err = currentScript;
		std::cerr << "failde to load string-based script in the program" << err.what() << std::endl;
		*/
		// TODO handle errors
	}

	sol::protected_function_result currentScriptResult = currentScript();
}

void LuaScripting::endScript() {
	scriptLoaded = false;
}

void LuaScripting::callLuaMainloop() {
	// TODO how to handle the main loop
}