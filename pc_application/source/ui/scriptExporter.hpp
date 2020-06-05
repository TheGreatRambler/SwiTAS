#pragma once

#include <rapidjson/document.h>
#include <string>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"
#include "../dataHandling/projectHandler.hpp"
#include "../helpers.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"

class ScriptExporter : public wxDialog {
private:
	std::shared_ptr<ProjectHandler> projectHandler;
	std::string dataToSave;

public:
	ScriptExporter(std::shared_ptr<ProjectHandler> projHandler, std::string data);
};