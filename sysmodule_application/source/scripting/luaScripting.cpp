#include "luaScripting.hpp"

LuaScripting::LuaScripting(std::shared_ptr<Syscalls> syscalls) {
	// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html
	// https://github.com/yuzu-emu/yuzu/wiki/Building-for-Windows
	// https://sol2.readthedocs.io/en/latest/api/function.html
	// https://sol2.readthedocs.io/en/latest/tutorial/all-the-things.html
	yuzuSyscalls = syscalls;

	luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine, sol::lib::string, sol::lib::os, sol::lib::math, sol::lib::table, sol::lib::bit32, sol::lib::io, sol::lib::utf8);
}

void LuaScripting::loadScript(std::string path) {
	luaPath = path;

	luaExecutionThread = std::make_unique<std::thread>(&LuaScripting::luaThread, this);
}

void LuaScripting::endScript() {
	scriptLoaded = false;
}

void LuaScripting::luaThread() {
	sol::load_result currentScript = luaState.load_file(luaPath);
	if(!currentScript.valid()) {
		/*
		sol::error err = currentScript;
		std::cerr << "failde to load string-based script in the program" << err.what() << std::endl;
		*/
		// TODO handle errors
	}

	sol::protected_function_result currentScriptResult = currentScript();
}

void LuaScripting::sendSyscall(std::function<void()> func) {
	syscallReady = true;
	std::unique_lock<std::mutex> lk(syscallMutex);
	syscallCv.wait(lk, [this] { return ready; });

	// Run the syscalls provided by yuzu or otherwise
	func();
	processed    = true;
	syscallReady = false;

	lk.unlock();
	syscallCv.notify_one();
}

void LuaScripting::callMainloop() {
	// https://en.cppreference.com/w/cpp/thread/condition_variable
	// 10 syscalls handled a time
	for(uint8_t i = 0; i < 10; i++) {
		// Somehow, inform syscall (if a syscall is waiting at the moment) through the condition variabke
		// Continue if a syscall is avalible at the beginning of this loop and break otherwise
		// A max of 10 syscalls will be handled at a time
		if(syscallReady) {
			{
				std::lock_guard<std::mutex> lk(syscallMutex);
				ready = true;
			}
			syscallCv.notify_one();
			// wait for lua to run syscalls

			{
				// Gain back control
				std::unique_lock<std::mutex> lk(syscallMutex);
				syscallCv.wait(lk, [this] { return processed; });
			}
		} else {
			break;
		}
	}
}