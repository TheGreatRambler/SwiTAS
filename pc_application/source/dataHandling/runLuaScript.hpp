#pragma once

#include <sol.hpp>

#include "../sharedNetworkCode/networkInterface.hpp"
#include "dataProcessing.hpp"
#include "projectHandler.hpp"

// MAKE THIS INTO A DIALOG
class RunLuaScript {
private:
	std::string scriptPath;
	bool scriptExists = false;

	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	DataProcessing* inputInstance;

	sol::state luaState;

public:
	RunLuaScript(std::shared_ptr<ProjectHandler> projHandler, std::shared_ptr<CommunicateWithNetwork> networkImp, DataProcessing* input);

	void setPath(std::string path) {
		scriptPath   = path;
		scriptExists = true;
	}
};