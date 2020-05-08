#pragma once

// clang-format off
#define ADD_NETWORK_CALLBACK_MAP(Flag) std::unordered_map<uint8_t, \
	std::function<void(const Protocol::Struct_##Flag&)>> Callbacks_##Flag;
// clang-format on

// clang-format off
// https://stackoverflow.com/a/20583578/9329945
#define ADD_NETWORK_CALLBACK(Flag, callbackBody) { \
	std::function<void(const Protocol::Struct_##Flag&)> func = \
		[this] (const Protocol::Struct_##Flag& data) { \
			callbackBody \
		}; \
	projectHandler->Callbacks_##Flag.insert(std::make_pair<uint8_t, \
		std::function<void(const Protocol::Struct_##Flag&)>>(std::move(NETWORK_CALLBACK_ID), std::move(func))); \
}
// clang-format on

#define REMOVE_NETWORK_CALLBACK(Flag) projectHandler->Callbacks_##Flag.erase(NETWORK_CALLBACK_ID);

// Comes from the network code
// clang-format off
#define PROCESS_NETWORK_CALLBACKS(networkInstance, Flag) { \
	Protocol::Struct_##Flag data; \
	while (networkInstance->Queue_##Flag.try_dequeue(data)) { \
		for (auto const& callback : projectHandler->Callbacks_##Flag) { \
			callback.second(data); \
		} \
	} \
}
// clang-format on

#include <fstream>
#include <functional>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <unordered_map>
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
	wxFrame* parentFrame;

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

	void closeVideoComparisonViewer(VideoComparisonViewer* viewer);
	void updateVideoComparisonViewers(int delta);

public:
	ProjectHandler(wxFrame* parent, DataProcessing* dataProcessingInstance, rapidjson::Document* settings);

	ADD_NETWORK_CALLBACK_MAP(RecieveFlag)
	ADD_NETWORK_CALLBACK_MAP(RecieveGameInfo)
	ADD_NETWORK_CALLBACK_MAP(RecieveGameFramebuffer)
	ADD_NETWORK_CALLBACK_MAP(RecieveApplicationConnected)
	ADD_NETWORK_CALLBACK_MAP(RecieveLogging)
	ADD_NETWORK_CALLBACK_MAP(RecieveMemoryRegion)

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

class NetworkCallbackHandler {
private:
public:
	NetworkCallbackHandler();
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
	bool isNewProject           = false;

	bool projectChosen = false;

	void onClickProject(wxCommandEvent& event);

public:
	ProjectHandlerWindow(std::shared_ptr<ProjectHandler> projHandler, rapidjson::Document* settings);

	uint8_t wasDialogClosedForcefully() {
		return wasClosedForcefully;
	}

	bool wasProjectChosen() {
		return projectChosen;
	}

	bool loadedNewProject() {
		return isNewProject;
	}

	// Created if no project is chosen
	void createTempProjectDir();
};