#pragma once

#include <cstdint>
#include <memory>
#include <mio.hpp>
#include <system_error>
#include <vector>
#include <wx/filepicker.h>
#include <wx/wx.h>

#include "../dataHandling/projectHandler.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"
#include "../sharedNetworkCode/networkingStructures.hpp"

// This will use a wxListCtrl to list the memory locations currently shown
// You will be able to add values by using their memory viewer fancy string version
// The type will be specified and the data will be exported to a file on demand

struct MemoryItemInfo {
	uint8_t isUnsigned;
	MemoryRegionTypes type;
	uint16_t size;
	uint8_t saveToFile;
	wxString filePath;
	wxString pointerPath;
};

class MemoryViewer : public wxFrame {
private:
	wxBoxSizer* mainSizer;
	wxBoxSizer* entryEditerSizer;

	uint16_t currentItemSelection = 0;

	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInterface;

	std::vector<MemoryItemInfo> infos;

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

	wxListCtrl* itemsList;

	void onUpdateEntry(wxCommandEvent& event);
	void onAddEntry(wxCommandEvent& event);

	void selectedItemChanged(wxListEvent& event);

public:
	MemoryViewer(wxFrame* parent, std::shared_ptr<ProjectHandler> proj, std::shared_ptr<CommunicateWithNetwork> networkImp);
};