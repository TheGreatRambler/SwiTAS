#pragma once

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/grid.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/wx.h>

#include "dataProcessing.hpp"

class ProjectHandler : public wxFrame {
private:
	// Add directory of project, project name, various other info
	// This will handle opening a directory chooser at the beginning of each session
	// https://docs.wxwidgets.org/3.0/classwx_dir_dialog.html
	// Probably having a list of the recent projects as well
	// Store recent projects in mainSettings.json
	wxBoxSizer* mainSizer;
	wxRichTextCtrl* projectList;

	DataProcessing* dataProcessing;

	wxDir projectDir;

	// Main settings variable
	rapidjson::Document mainSettings;

	void getGlobalSettings();

	void onClickProject(wxTextUrlEvent& event);

public:
	ProjectHandler(DataProcessing* dataProcessingInstance);

	void saveProject();
};