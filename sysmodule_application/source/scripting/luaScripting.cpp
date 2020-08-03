#include "luaScripting.hpp"

LuaScripting::LuaScripting() {
	// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html
	// https://github.com/yuzu-emu/yuzu/wiki/Building-for-Windows
	// https://sol2.readthedocs.io/en/latest/api/function.html
	// https://sol2.readthedocs.io/en/latest/tutorial/all-the-things.html
	luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine, sol::lib::string, sol::lib::os, sol::lib::math, sol::lib::table, sol::lib::bit32, sol::lib::io, sol::lib::utf8);

	registerLuaFuncs();
}

void LuaScripting::registerLuaFuncs() {
	// TODO not super important right now
	luaState.set_function("emu_is_closing", [this]() -> bool { return isClosing; });
}

void LuaScripting::loadScript(std::string path) {
	luaPath = path;

	sol::load_result currentScript = luaState.load_file(luaPath);
	if(!currentScript.valid()) {
		/*
		sol::error err = currentScript;
		std::cerr << "failde to load string-based script in the program" << err.what() << std::endl;
		*/
		// TODO handle errors
	}

	sol::protected_function_result currentScriptResult = currentScript();

	sol::protected_function init = luaState["init"];
	if(init.valid()) {
		init();
	}
}

void LuaScripting::endScript() {
	sol::protected_function quit = luaState["quit"];
	if(quit.valid()) {
		quit();
	}
}

void LuaScripting::callMainloop() {
	sol::protected_function main_loop = luaState["main_loop"];
	if(main_loop.valid()) {
		main_loop();
	}
}