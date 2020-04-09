#pragma once

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>
#include <wx/grid.h>
#include <wx/listbox.h>
#include <wx/mstream.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/wx.h>
#include <wx/zstream.h>

#include "../../sharedNetworkCode/serializeUnserializeData.hpp"
#include "dataProcessing.hpp"

class ProjectHandler : public wxDialog {
private:
	// Add directory of project, project name, various other info
	// This will handle opening a directory chooser at the beginning of each session
	// https://docs.wxwidgets.org/3.0/classwx_dir_dialog.html
	// Probably having a list of the recent projects as well
	// Store recent projects in mainSettings.json
	wxBoxSizer* mainSizer;
	wxListBox* projectList;

	DataProcessing* dataProcessing;

	wxDir projectDir;
	SerializeProtocol serializeProtocol;
	bool projectChosen = false;

	std::string projectName;

	int recentProjectChoice;

	const wxString loadExistingProjectText = "Load Project";
	const wxString createNewProjectText    = "Create New Project";

	static constexpr int compressionLevel = 7;

	// Main settings variable
	rapidjson::Document* mainSettings;

	void onClickProject(wxCommandEvent& event);

	wxFileName getProjectStart() {
		return wxFileName::DirName(projectDir.GetNameWithSep());
	}

public:
	ProjectHandler(DataProcessing* dataProcessingInstance, rapidjson::Document* settings);

	void loadProject();
	void saveProject();

	// Created if no project is chosen
	void createTempProjectDir();

	bool wasProjectChosen() {
		return projectChosen;
	}

	void setProjectName(std::string name) {
		projectName = name;
	}

	std::string getProjectName() {
		return projectName;
	}

	~ProjectHandler();
};