#include "runLuaScript.hpp"

RunLuaScript::RunLuaScript(std::shared_ptr<ProjectHandler> projHandler, std::shared_ptr<CommunicateWithNetwork> networkImp, DataProcessing* input) {
	projectHandler  = projHandler;
	networkInstance = networkImp;
	inputInstance   = input;

	luaState.open_libraries(sol::lib::base);

	luaState["dataProcessing"] = inputInstance;

	// http://www.fceux.com/web/help/fceux.html?LuaScripting.html
	// https://github.com/ThePhD/sol2/blob/develop/examples/source/tutorials/quick_n_dirty/userdata_memory_reference.cpp
	// Enable dataProcessing as a lua type, TODO add public calls for lua to use
	// luaState.new_usertype<DataProcessing>("DataProcessing",
	//"tailwag", &Doge::tailwag
	//);
}