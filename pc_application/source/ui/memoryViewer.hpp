#pragma once

#include <cstdint>
#include <memory>
#include <mio.hpp>
#include <system_error>
#include <wx/filepicker.h>
#include <wx/wx.h>

#include "../dataHandling/projectHandler.hpp"
#include "../sharedNetworkCode/networkInterface.hpp"
#include "../sharedNetworkCode/networkingStructures.hpp"

// This will use a wxListCtrl to list the memory locations currently shown
// You will be able to add values by using their memory viewer fancy string version
// The type will be specified and the data will be exported to a file on demand
class MemoryViewer : public wxFrame {
private:
	wxBoxSizer* mainSizer;
	wxBoxSizer* entryEditerSizer;

	std::shared_ptr<ProjectHandler> projectHandler;
	std::shared_ptr<CommunicateWithNetwork> networkInterface;

	wxCheckBox* unsignedCheckbox;
	// Doesn't apply because the switch is little endian
	// wxCheckBox* littleEndianCheckbox;
	wxChoice* typeSelection;
	// Used only for byte arrays and char strings, hidden for all others
	wxSpinCtrl* itemSize;
	wxCheckBox* saveToFile;
	wxFilePickerCtrl* filePath;

	wxButton* updateEntry;
	wxButton* addEntry;

	void typeChanged(wxCommandEvent& event);

	void onUpdateEntry(wxCommandEvent& event);
	void onAddEntry(wxCommandEvent& event);

public:
	MemoryViewer(wxFrame* parent, std::shared_ptr<ProjectHandler> proj, std::shared_ptr<CommunicateWithNetwork> networkImp);
};