#pragma once

#include <cstdio>
#include <rapidjson/document.h>
#include <string>
#include <vector>
#include <wx/filepicker.h>
#include <wx/sstream.h>
#include <wx/stream.h>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"
#include "../dataHandling/projectHandler.hpp"
#include "../helpers.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"
#include "../thirdParty/FTP/FTPClient.h"

class ScriptExporter : public wxDialog {
private:
	std::shared_ptr<ProjectHandler> projectHandler;
	std::string dataToSave;

	wxBoxSizer* mainSizer;

	// Press enter to submit
	wxTextCtrl* ftpEntry;

	// For the current filesystem
	wxButton* selectForFilesystem;

	std::string ftpAddress;

	void onFtpSelect(wxCommandEvent& event);
	void onFilesystemOpen(wxCommandEvent& event);

public:
	ScriptExporter(wxFrame* parent, std::shared_ptr<ProjectHandler> projHandler, std::string data);
};