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
	const uint8_t NETWORK_CALLBACK_ID = 4;

	wxBoxSizer* mainSizer;
	wxBoxSizer* entryEditerSizer;

	long currentItemSelection = 0;

	wxString typeChoices[MemoryRegionTypes::NUM_OF_TYPES];

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
	wxButton* removeEntry;

	wxListCtrl* itemsList;

	void onUpdateEntry(wxCommandEvent& event);
	void onAddEntry(wxCommandEvent& event);
	void onRemoveEntry(wxCommandEvent& event);

	void onIdle(wxIdleEvent& event);
	void onClose(wxCloseEvent& event);

	void sendUpdatedEntries();

	void selectedItemChanged(wxListEvent& event);

public:
	MemoryViewer(wxFrame* parent, std::shared_ptr<ProjectHandler> proj, std::shared_ptr<CommunicateWithNetwork> networkImp);

	DECLARE_EVENT_TABLE();
};