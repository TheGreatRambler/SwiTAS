#include "scriptExporter.hpp"

ScriptExporter::ScriptExporter(wxFrame* parent, std::shared_ptr<ProjectHandler> projHandler, std::string data)
	: wxDialog(parent, wxID_ANY, "Savestate Selection", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE) {
	projectHandler = projHandler;
	dataToSave     = data;

	// TODO
	// FTP path (standard format)
	// Submit button for FTP (reject if invalid path)

	// Filesystem selector (simple file selector, allow creation)

	mainSizer = new wxBoxSizer(wxVERTICAL);

	std::string lastEnteredFtpPath = projectHandler->getLastEnteredFtpPath();

	wxStaticText* ftpLabel = new wxStaticText(this, wxID_ANY, "Export to FTP server");
	ftpEntry               = new wxTextCtrl(this, wxID_ANY, wxString::FromUTF8(lastEnteredFtpPath), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_CENTRE);

	wxStaticText* filesystemLabel = new wxStaticText(this, wxID_ANY, "Export to local filesystem");
	selectForFilesystem           = new wxButton(this, wxID_ANY, "Select filename");

	ftpEntry->Bind(wxEVT_TEXT_ENTER, &ScriptExporter::onFtpSelect, this);
	selectForFilesystem->Bind(wxEVT_BUTTON, &ScriptExporter::onFilesystemOpen, this);

	mainSizer->Add(ftpLabel, 0, wxEXPAND | wxALL);
	mainSizer->Add(ftpEntry, 0, wxEXPAND | wxALL);
	mainSizer->Add(filesystemLabel, 0, wxEXPAND | wxALL);
	mainSizer->Add(selectForFilesystem, 0, wxEXPAND | wxALL);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void ScriptExporter::onFtpSelect(wxCommandEvent& event) {
	// The thing passed is a ftp address, parse it
	// wxFTP ftp;
	ftpAddress = ftpEntry->GetLineText(0).ToStdString();

	wxString tempPath = wxFileName::CreateTempFileName("script");
	wxFile file(tempPath, wxFile::write);
	file.Write(wxString::FromUTF8(dataToSave));
	file.Close();

	std::string output = HELPERS::exec(wxString::Format("curl -T %s -m 10 --connect-timeout 3 --verbose %s", tempPath, ftpAddress).c_str());

	wxRemoveFile(tempPath);

	if(output.find("is not recognized") != std::string::npos || output.find("command not found") != std::string::npos) {
		wxMessageDialog errorDialog(this, "Curl Not Found", "The Curl executable was not found", wxOK | wxICON_ERROR);
		errorDialog.ShowModal();
	} else if(output.find("Closing connection 0") != std::string::npos) {
		std::vector<std::string> lines = HELPERS::splitString(output, '\n');

		wxMessageDialog errorDialog(this, "Curl Error", wxString::FromUTF8(lines[lines.size() - 1]), wxOK | wxICON_ERROR);
		errorDialog.ShowModal();
	} else {
		EndModal(wxID_OK);
	}
}

void ScriptExporter::onFilesystemOpen(wxCommandEvent& event) {
	wxFileDialog saveFileDialog(this, _("Save Script file"), "", "", "Text files (*.txt)|*.txt|nx-TAS script files (*.ssctf)|*.ssctf", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	// Default directory is in the project folder
	saveFileDialog.SetDirectory(projectHandler->getProjectStart().GetFullPath());

	if(saveFileDialog.ShowModal() == wxID_OK) {
		wxFile file(saveFileDialog.GetPath(), wxFile::write);
		file.Write(wxString::FromUTF8(dataToSave));
		file.Close();

		EndModal(wxID_OK);
	}
}