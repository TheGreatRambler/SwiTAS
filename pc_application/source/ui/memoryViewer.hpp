#pragma once

#include <cstdint>
#include <memory>
#include <mio.hpp>
#include <system_error>
#include <vector>
#include <wx/filepicker.h>
#include <wx/wfstream.h>
#include <wx/wx.h>

#include "../dataHandling/projectHandler.hpp"
#include "drawingCanvas.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"
#include "../sharedNetworkCode/networkingStructures.hpp"

struct MemoryItemInfo {
	uint8_t isUnsigned;
	MemoryRegionTypes type;
	uint16_t size;
	uint8_t saveToFile;
	wxString filePath;
	wxString pointerPath;
	mio::mmap_sink mmap;
};

class MemorySectionViewer : public DrawingCanvas {
private:
std::vector<MemoryDataInfo::MemoryInfo> memoryInfo;

public:
	MemorySectionViewer(wxFrame* parent);

	void setMemoryInfo(std::vector<MemoryDataInfo::MemoryInfo> memInfo) {
memoryInfo = memInfo;
	}

	virtual void draw(wxDC& dc) override;
};

class MemoryViewer : public wxFrame {
private:
	const uint8_t NETWORK_CALLBACK_ID = 4;

	wxBoxSizer* mainSizer;
	wxBoxSizer* entryEditerSizer;

	long currentItemSelection = 0;

	wxString typeChoices[MemoryRegionTypes::NUM_OF_TYPES];

	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInterface;

	std::vector<MemoryItemInfo> infos;

	std::error_code errorCode;

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

	void sendUpdatedEntries();

	void mapFile(MemoryItemInfo& info) {
		if(info.saveToFile) {
			wxRemoveFile(info.filePath);

			wxFile theFile;
			// Allow reading and writing by all users
			theFile.Create(info.filePath, true, wxS_DEFAULT);
			// Triggers sparse file creation to get the file created at the right size
			// https://stackoverflow.com/questions/7896035/c-make-a-file-of-a-specific-size
			theFile.Seek(info.size - 1);
			theFile.Write("", 1);
			theFile.Close();

			// Map this file as memory
			// https://github.com/mandreyel/mio
			info.mmap = mio::make_mmap_sink(info.filePath.ToStdString(), 0, mio::map_entire_file, errorCode);
		}
	}

	void selectedItemChanged(wxListEvent& event);

public:
	MemoryViewer(wxFrame* parent, std::shared_ptr<ProjectHandler> proj, std::shared_ptr<CommunicateWithNetwork> networkImp);

	DECLARE_EVENT_TABLE();
};