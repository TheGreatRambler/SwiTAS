#pragma once

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <wx/filepicker.h>
#include <wx/fswatcher.h>
#include <wx/wx.h>

#include "../data_handling/projectHandler.hpp"
#include "../shared_network_code/networkInterface.hpp"
#include "../shared_network_code/networkingStructures.hpp"
#include "drawingCanvas.hpp"

struct MemoryItemInfo {
	uint8_t isUnsigned = false;
	MemoryRegionTypes type;
	uint16_t size        = 0;
	uint8_t saveToFile   = false;
	wxString filePath    = "";
	wxString pointerPath = "";
};

class MemorySectionViewer : public DrawingCanvas {
private:
	std::vector<MemoryDataInfo::MemoryInfo> memoryInfo;
	uint64_t totalMemorySize = 0;

	rapidjson::Document* mainSettings;

	std::unordered_map<MemoryDataInfo::MemoryType, wxBrush> sectionColors;

	void onMouseMove(wxMouseEvent& event);

public:
	MemorySectionViewer(wxWindow* parent, rapidjson::Document* settings);

	void setMemoryInfo(std::vector<MemoryDataInfo::MemoryInfo> memInfo) {
		memoryInfo = memInfo;
		for(auto const& region : memoryInfo) {
			totalMemorySize += region.size;
		}
	}

	virtual void draw(wxDC& dc) override;

	DECLARE_EVENT_TABLE();
};

class MemoryViewer : public wxFrame {
private:
	const uint8_t NETWORK_CALLBACK_ID = 4;

	wxBoxSizer* mainSizer;
	wxBoxSizer* entryEditerSizer;

	long currentItemSelection = 0;

	wxString typeChoices[(uint8_t)MemoryRegionTypes::NUM_OF_TYPES];

	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInterface;

	std::vector<MemoryItemInfo> infos;

	wxFileSystemWatcher fileSystemWatcher;

	wxCheckBox* unsignedCheckbox;
	// Doesn't apply because the switch is little endian
	// wxCheckBox* littleEndianCheckbox;
	wxChoice* typeSelection;
	// Used only for byte arrays and char strings, hidden for all others
	wxSpinCtrl* itemSize;
	wxCheckBox* saveToFile;
	wxFilePickerCtrl* filePath;

	wxTextCtrl* pointerPath;

	wxButton* updateEntry;
	wxButton* addEntry;
	wxButton* removeEntry;

	wxListCtrl* itemsList;

	void onUpdateEntry(wxCommandEvent& event);
	void onAddEntry(wxCommandEvent& event);
	void onRemoveEntry(wxCommandEvent& event);

	void onIdle(wxIdleEvent& event);
	void onClose(wxCloseEvent& event);

	void addFromVector(std::vector<MemoryItemInfo> vec);
	std::vector<MemoryItemInfo>& getVector();

	void sendUpdatedEntries();

	void mapFile(MemoryItemInfo& info);

	void selectedItemChanged(wxListEvent& event);
	void updateAtIndex(long index);
	void beginLabelEdit(wxListEvent& event);
	void finishLabelEdit(wxListEvent& event);

	void fileChangesDetected(wxFileSystemWatcherEvent& event);

public:
	MemoryViewer(wxFrame* parent, std::shared_ptr<ProjectHandler> proj, std::shared_ptr<CommunicateWithNetwork> networkImp);

	DECLARE_EVENT_TABLE();
};