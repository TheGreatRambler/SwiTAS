#include "runFinalTas.hpp"

TasRunner::TasRunner(wxFrame* parent, std::shared_ptr<CommunicateWithNetwork> networkImp, rapidjson::Document* settings, DataProcessing* inputData)
	: wxDialog(parent, wxID_ANY, "Run Final TAS", wxDefaultPosition, wxDefaultSize) {
	networkInstance = networkImp;
	mainSettings    = settings;
	dataProcessing  = inputData;

	mainSizer          = new wxBoxSizer(wxVERTICAL);
	hookSelectionSizer = new wxBoxSizer(wxHORIZONTAL);

	// TODO add everything else
}

void TasRunner::onStartTasHomebrewPressed(wxCommandEvent& event) {}

void TasRunner::onStartTasArduinoPressed(wxCommandEvent& event) {}