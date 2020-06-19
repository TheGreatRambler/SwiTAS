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
	typeChoices[MemoryRegionTypes::CharPointer] = "Char String";
	typeChoices[MemoryRegionTypes::ByteArray]   = "Byte Array";

	typeSelection = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MemoryRegionTypes::NUM_OF_TYPES, typeChoices);

	updateEntry = new wxButton(this, wxID_ANY, "Update Entry");
	addEntry    = new wxButton(this, wxID_ANY, "Add Entry");
}