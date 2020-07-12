#include "memoryViewer.hpp"

MemoryViewer::MemoryViewer(wxFrame* parent, std::shared_ptr<ProjectHandler> proj, std::shared_ptr<CommunicateWithNetwork> networkImp)
	: wxFrame(parent, wxID_ANY, "Memory Viewer", wxDefaultPosition, wxSize(300, 200), wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT) {
	projectHandler   = proj;
	networkInterface = networkImp;

	mainSizer        = new wxBoxSizer(wxHORIZONTAL);
	entryEditerSizer = new wxBoxSizer(wxVERTICAL);

	unsignedCheckbox = new wxCheckBox(this, wxID_ANY, "Unsigned");
	unsignedCheckbox->SetValue(true);

	wxString typeChoices[MemoryRegionTypes::NUM_OF_TYPES];

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

	pointerPath = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);

	updateEntry = new wxButton(this, wxID_ANY, "Update Entry");
	addEntry    = new wxButton(this, wxID_ANY, "Add Entry");

	updateEntry->Bind(wxEVT_BUTTON, &MemoryViewer::onUpdateEntry, this);
	addEntry->Bind(wxEVT_BUTTON, &MemoryViewer::onAddEntry, this);

	itemsList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);

	itemsList->Bind(wxEVT_LIST_ITEM_SELECTED, &MemoryViewer::selectedItemChanged, this);

	itemsList->InsertColumn(0, "Index", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	itemsList->InsertColumn(1, "Data Type", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	itemsList->InsertColumn(2, "Pointer Path", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	itemsList->InsertColumn(3, "Value", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
}

void MemoryViewer::selectedItemChanged(wxListEvent& event) {}

void MemoryViewer::onUpdateEntry(wxCommandEvent& event) {}

void MemoryViewer::onAddEntry(wxCommandEvent& event) {
	MemoryItemInfo info;

	info.isUnsigned;
	info.type;
	info.size;
	info.saveToFile;
	info.filePath;
	info.pointerPath;

	infos.push_back(info);
}