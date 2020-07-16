#include "memoryViewer.hpp"

MemoryViewer::MemoryViewer(wxFrame* parent, std::shared_ptr<ProjectHandler> proj, std::shared_ptr<CommunicateWithNetwork> networkImp)
	: wxFrame(parent, wxID_ANY, "Memory Viewer", wxDefaultPosition, wxSize(300, 200), wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT) {
	projectHandler   = proj;
	networkInterface = networkImp;

	mainSizer        = new wxBoxSizer(wxHORIZONTAL);
	entryEditerSizer = new wxBoxSizer(wxVERTICAL);

	unsignedCheckbox = new wxCheckBox(this, wxID_ANY, "Unsigned");
	unsignedCheckbox->SetValue(true);

	typeChoices[MemoryRegionTypes::Bit8]        = "8 Bit Number";
	typeChoices[MemoryRegionTypes::Bit16]       = "16 Bit Number";
	typeChoices[MemoryRegionTypes::Bit32]       = "32 Bit Number";
	typeChoices[MemoryRegionTypes::Bit64]       = "64 Bit Number";
	typeChoices[MemoryRegionTypes::Float]       = "Float";
	typeChoices[MemoryRegionTypes::Double]      = "Double";
	typeChoices[MemoryRegionTypes::Bool]        = "Bool";
	typeChoices[MemoryRegionTypes::CharPointer] = "Char String";
	typeChoices[MemoryRegionTypes::ByteArray]   = "Byte Array";

	typeSelection = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MemoryRegionTypes::NUM_OF_TYPES, typeChoices);
	typeSelection->SetSelection(0);

	pointerPath = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);

	itemSize = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, INT_MAX, 1);

	saveToFile = new wxCheckBox(this, wxID_ANY, "Save to file");
	saveToFile->SetValue(false);

	filePath = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, "Choose file to save this memory region in", wxFileSelectorPromptStr, wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT);

	updateEntry = new wxButton(this, wxID_ANY, "Update Entry");
	addEntry    = new wxButton(this, wxID_ANY, "Add Entry");
	removeEntry = new wxButton(this, wxID_ANY, "Remove Entry");

	updateEntry->Bind(wxEVT_BUTTON, &MemoryViewer::onUpdateEntry, this);
	addEntry->Bind(wxEVT_BUTTON, &MemoryViewer::onAddEntry, this);

	entryEditerSizer->Add(unsignedCheckbox, 0);
	entryEditerSizer->Add(typeSelection, 0);
	entryEditerSizer->Add(pointerPath, 0);
	entryEditerSizer->Add(itemSize, 0);
	entryEditerSizer->Add(saveToFile, 0);
	entryEditerSizer->Add(filePath, 0);
	entryEditerSizer->Add(updateEntry, 1, wxEXPAND | wxALL);
	entryEditerSizer->Add(addEntry, 1, wxEXPAND | wxALL);

	mainSizer->Add(entryEditerSizer, 1, wxEXPAND | wxALL);

	itemsList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);

	itemsList->Bind(wxEVT_LIST_ITEM_SELECTED, &MemoryViewer::selectedItemChanged, this);

	itemsList->InsertColumn(0, "Pointer Path", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	itemsList->InsertColumn(1, "Data Type", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	itemsList->InsertColumn(2, "Value", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);

	mainSizer->Add(itemsList, 1, wxEXPAND | wxALL);

	ADD_NETWORK_CALLBACK(RecieveMemoryRegion, {
		MemoryItemInfo& info = infos[data.index];

		if(info.type != MemoryRegionTypes::ByteArray) {
			// Byte arrays can't be represented this way
			itemsList->SetItem(data.index, 2, wxString::FromUTF8(data.stringRepresentation));
		}

		if(info.saveToFile) {
			std::copy(data.memory.begin(), data.memory.end(), info.mmap.begin());
			info.mmap.sync(errorCode);
		}
	})

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

// clang-format off
BEGIN_EVENT_TABLE(MemoryViewer, wxFrame)
    EVT_IDLE(MemoryViewer::onIdle)
	EVT_CLOSE(MemoryViewer::onClose)
END_EVENT_TABLE()
// clang-format on

void MemoryViewer::onIdle(wxIdleEvent& event) {
	PROCESS_NETWORK_CALLBACKS(networkInterface, RecieveMemoryRegion)
}

void MemoryViewer::onClose(wxCloseEvent& event) {
	REMOVE_NETWORK_CALLBACK(RecieveMemoryRegion)
}

void MemoryViewer::selectedItemChanged(wxListEvent& event) {
	MemoryItemInfo& info = infos[event.GetIndex()];

	unsignedCheckbox->SetValue(info.isUnsigned);
	typeSelection->SetSelection(info.type);
	itemSize->SetValue(info.size);
	saveToFile->SetValue(info.saveToFile);
	filePath->SetPath(info.filePath);
	pointerPath->SetValue(info.pointerPath);
}

void MemoryViewer::onUpdateEntry(wxCommandEvent& event) {
	long selectedItem = itemsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(selectedItem != wxNOT_FOUND) {
		// An item is selected and now it can be updated
		MemoryItemInfo& info = infos[selectedItem];

		info.isUnsigned  = unsignedCheckbox->GetValue();
		info.type        = (MemoryRegionTypes)typeSelection->GetCurrentSelection();
		info.size        = itemSize->GetValue();
		info.saveToFile  = saveToFile->GetValue();
		info.filePath    = filePath->GetPath();
		info.pointerPath = pointerPath->GetLineText(0);

		mapFile(info);

		itemsList->SetItem(selectedItem, 0, info.pointerPath);
		itemsList->SetItem(selectedItem, 1, typeChoices[info.type]);

		sendUpdatedEntries();
	}
}

void MemoryViewer::onAddEntry(wxCommandEvent& event) {
	MemoryItemInfo info;

	info.isUnsigned  = unsignedCheckbox->GetValue();
	info.type        = (MemoryRegionTypes)typeSelection->GetCurrentSelection();
	info.size        = itemSize->GetValue();
	info.saveToFile  = saveToFile->GetValue();
	info.filePath    = filePath->GetPath();
	info.pointerPath = pointerPath->GetLineText(0);

	mapFile(info);

	infos.push_back(std::move(info));

	long itemIndex = itemsList->InsertItem(0, info.pointerPath);
	itemsList->SetItem(itemIndex, 1, typeChoices[info.type]);
	// 3rd column stays empty

	sendUpdatedEntries();
}

void MemoryViewer::onRemoveEntry(wxCommandEvent& event) {
	long selectedItem = itemsList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(selectedItem != wxNOT_FOUND) {
		itemsList->DeleteItem(selectedItem);
		itemsList->Refresh();

		infos.erase(infos.begin() + selectedItem);

		sendUpdatedEntries();
	}
}

void MemoryViewer::sendUpdatedEntries() {
	// clang-format off
	ADD_TO_QUEUE(SendAddMemoryRegion, networkInterface, {
		data.clearAllRegions = true;
	})
	// clang-format on

	for(auto const& item : infos) {
		ADD_TO_QUEUE(SendAddMemoryRegion, networkInterface, {
			data.pointerDefinition = item.pointerPath.ToStdString();
			data.type              = item.type;
			data.clearAllRegions   = false;
			data.u                 = item.isUnsigned;
			data.dataSize          = item.size;
		})
	}
}