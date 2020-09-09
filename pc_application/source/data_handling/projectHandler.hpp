#pragma once

// clang-format off
#define ADD_NETWORK_CALLBACK_MAP(Flag) std::unordered_map<uint8_t, \
	std::function<void(const Protocol::Struct_##Flag&)>> Callbacks_##Flag;
// clang-format on

// clang-format off
// https://stackoverflow.com/a/20583578/9329945
#define ADD_NETWORK_CALLBACK(Flag, callbackBody) { \
	projectHandler->Callbacks_##Flag.emplace(NETWORK_CALLBACK_ID, [this] (const Protocol::Struct_##Flag& data) { \
		callbackBody \
	}); \
}
// clang-format on

#define REMOVE_NETWORK_CALLBACK(Flag) projectHandler->Callbacks_##Flag.erase(NETWORK_CALLBACK_ID);

// Comes from the network code
// clang-format off
#define PROCESS_NETWORK_CALLBACKS(networkInstance, Flag) { \
	Protocol::Struct_##Flag data; \
	while (networkInstance->Queue_##Flag.try_dequeue(data)) { \
		for (auto const& callback : projectHandler->Callbacks_##Flag) { \
			if (callback.first < 10) { \
				callback.second(data); \
			} \
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
#include <wx/xml/xml.h>
#include <wx/dirdlg.h>
#include <cstring>
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
#include <wx/sstream.h>
#include <wx/zipstrm.h>
#include <wx/buffer.h>
#include <zstd.h>

#include "../shared_network_code/serializeUnserializeData.hpp"
#include "../ui/drawingCanvas.hpp"
#include "../ui/videoComparisonViewer.hpp"
#include "dataProcessing.hpp"

class ProjectSettingsWindow : public wxFrame {
private:
	// Just in case we need to listen for values
	const uint8_t NETWORK_CALLBACK_ID = 5;

	std::shared_ptr<ProjectHandler> projectHandler;
	rapidjson::Document* mainSettings;
	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	wxBoxSizer* mainSizer;

	wxBoxSizer* gameNameSizer;
	wxTextCtrl* gameTitleIdEntry;
	wxStaticText* gameName;

	wxBoxSizer* isMobileSizer;
	wxCheckBox* isMobile;

	wxTextCtrl* getLabel(wxString name) {
		return new wxTextCtrl(this, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_CENTRE);
	}

	void onTitleIdEntry(wxCommandEvent& event);
	void onChangeDocked(wxCommandEvent& event);

	void onClose(wxCloseEvent& event);

public:
	ProjectSettingsWindow(wxFrame* parentFrame, std::shared_ptr<ProjectHandler> projHandler, rapidjson::Document* settings, std::shared_ptr<CommunicateWithNetwork> network);
};

class ProjectHandler {
private:
	DataProcessing* dataProcessing;
	wxFrame* parentFrame;

	// XML parser for game database
	wxXmlDocument gameDatabaseXML;

	wxFileName projectDir;
	SerializeProtocol serializeProtocol;

	std::string projectName;
	uint8_t projectWasLoaded = true;

	uint16_t rerecordCount = 0;

	int recentProjectChoice;

	// For file exporting
	std::string lastEnteredFtpPath;

	static constexpr int compressionLevel = 7;

	// Main settings variable
	rapidjson::Document* mainSettings;
	rapidjson::Document recentSettings;

	wxString titleID = wxEmptyString;
	uint8_t isMobile = false;

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

	void promptForUpdate();

	void newProjectWasCreated();

	wxFileName getProjectStart() {
		return wxFileName::DirName(projectDir.GetPathWithSep());
	}

	wxXmlNode* getGameInfoFromTitleId(wxString titleId) {
		wxXmlNode* child = gameDatabaseXML.GetRoot()->GetNext()->GetChildren();
		while(child) {
			wxString id = child->GetAttribute("titleid");

			if(id == titleId) {
				return child;
			}

			child = child->GetNext();
		}

		return NULL;
	}

	std::string getLastEnteredFtpPath() {
		return lastEnteredFtpPath;
	}

	void setTitleId(wxString titleId) {
		titleID = titleId;
	}

	wxString getTitleId() {
		return titleID;
	}

	void setLastEnteredFtpPath(std::string path) {
		lastEnteredFtpPath = path;
	}

	void setProjectName(std::string name) {
		projectName = name;
		parentFrame->SetTitle("SwiTAS | " + wxString::FromUTF8(name));
	}

	void setRecentProjectChoice(int projectChoice) {
		recentProjectChoice = projectChoice;
	}

	void setProjectDir(wxString dirPath) {
		projectDir = wxFileName(dirPath, "");
		// Just in case
		projectDir.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	}

	void setProjectWasLoaded(bool wasLoaded) {
		projectWasLoaded = wasLoaded;
	}

	uint8_t getDocked() {
		return isMobile;
	}

	void setDocked(uint8_t val) {
		isMobile = val;
	}

	void removeRecentProject(int index) {
		recentSettings["recentProjects"].GetArray().Erase(&recentSettings["recentProjects"].GetArray()[index]);
	}

	std::string getProjectName() {
		return projectName;
	}

	wxMenu* getVideoSubmenu() {
		return videoComparisonEntriesMenu;
	}

	void incrementRerecordCount() {
		rerecordCount++;
	}

	void openUpVideoComparisonViewer(int index);

	void onRecentVideosMenuOpen(wxMenuEvent& event);

	rapidjson::GenericArray<false, rapidjson::Value> getRecentProjects() {
		return recentSettings["recentProjects"].GetArray();
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
	ProjectHandlerWindow(wxFrame* parent, std::shared_ptr<ProjectHandler> projHandler, rapidjson::Document* settings);

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