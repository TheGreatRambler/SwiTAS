#pragma once

#include <fstream>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>
#include <wx/grid.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include <wx/statbmp.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/wfstream.h>
#include <wx/wx.h>
#include <wx/zstream.h>

#include "../../sharedNetworkCode/serializeUnserializeData.hpp"
#include "../ui/drawingCanvas.hpp"
#include "../ui/videoComparisonViewer.hpp"
#include "dataProcessing.hpp"

class ProjectHandler {
private:
	DataProcessing* dataProcessing;

	wxDir projectDir;
	SerializeProtocol serializeProtocol;

	std::string projectName;
	uint8_t projectWasLoaded = true;

	int recentProjectChoice;

	static constexpr int compressionLevel = 7;

	// Main settings variable
	rapidjson::Document* mainSettings;

	std::vector<std::shared_ptr<VideoEntry>> videoComparisonEntries;
	wxMenu* videoComparisonEntriesMenu;

	// Video comparison frames open
	std::vector<VideoComparisonViewer*> videoComparisonViewers;

public:
	ProjectHandler(DataProcessing* dataProcessingInstance, rapidjson::Document* settings);

	void loadProject();
	void saveProject();

	wxFileName getProjectStart() {
		return wxFileName::DirName(projectDir.GetNameWithSep());
	}

	void setProjectName(std::string name) {
		projectName = name;
	}

	void setRecentProjectChoice(int projectChoice) {
		recentProjectChoice = projectChoice;
	}

	void setProjectDir(wxString dirPath) {
		projectDir.Open(dirPath);
		// Just in case
		wxFileName dir(dirPath);
		if(!dir.DirExists()) {
			dir.Mkdir();
		}
	}

	void setProjectWasLoaded(bool wasLoaded) {
		projectWasLoaded = wasLoaded;
	}

	void removeRecentProject(int index) {
		(*mainSettings)["recentProjects"].GetArray().Erase(&(*mainSettings)["recentProjects"].GetArray()[index]);
	}

	std::string getProjectName() {
		return projectName;
	}

	wxMenu* getVideoSubmenu() {
		return videoComparisonEntriesMenu;
	}

	void openUpVideoComparisonViewer(int index);

	void onRecentVideosMenuOpen(wxMenuEvent& event);

	rapidjson::GenericArray<false, rapidjson::Value> getRecentProjects() {
		return (*mainSettings)["recentProjects"].GetArray();
	}

	// Just a random large number, apparently can't be larger than 76
	static constexpr int videoComparisonEntriesMenuIDBase = 35;
};

class ProjectHandlerWindow : public wxDialog {
private:
	std::shared_ptr<ProjectHandler> projectHandler;

	rapidjson::Document* mainSettings;

	wxBoxSizer* mainSizer;
	wxListBox* projectList;

	const wxString loadExistingProjectText = "Load Project";
	const wxString createNewProjectText    = "Create New Project";

	uint8_t wasClosedForcefully = true;

	bool projectChosen = false;

#ifdef __WXGTK__
	bool projectListFirstTime = true;
#endif

	void onClickProject(wxCommandEvent& event);

public:
	ProjectHandlerWindow(std::shared_ptr<ProjectHandler> projHandler, rapidjson::Document* settings);

	uint8_t wasDialogClosedForcefully() {
		return wasClosedForcefully;
	}

	bool wasProjectChosen() {
		return projectChosen;
	}

	// Created if no project is chosen
	void createTempProjectDir();
};