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

	// https://github.com/embeddedmz/ftpclient-cpp
	embeddedmz::CFTPClient FTPClient([](const std::string& strLogMsg) {});

	// First, test ftp with user and password
	// http://jkorpela.fi/ftpurl.html
	char user[16];
	char password[100];
	char host[50];
	int port;
	char path[1024];

	uint8_t userSet     = false;
	uint8_t passwordSet = false;
	uint8_t portSet     = false;

	uint8_t successful = false;

	if(sscanf(ftpAddress.c_str(), "ftp://%[^:]:%[^@]@%[^:]:%d%s", user, password, host, &port, path) == 5) {
		userSet     = true;
		passwordSet = true;
		portSet     = true;

		successful = true;
	} else {
		if(sscanf(ftpAddress.c_str(), "ftp://%[^:]:%d%s", host, &port, path) == 3) {
			portSet = true;

			successful = true;
		} else {
			if(sscanf(ftpAddress.c_str(), "ftp://%[^/]%s", host, path) == 2) {
				successful = true;
			}
		}
	}

	if(successful) {
		if(portSet && userSet && passwordSet) {
			FTPClient.InitSession(std::string("ftp://") + host, port, std::string(user), std::string(password));
		} else if(portSet) {
			FTPClient.InitSession(std::string("ftp://") + host, port, "anonymous", "guest");
		} else {
			FTPClient.InitSession(std::string("ftp://") + host, 21, "anonymous", "guest");
		}

		std::string fullPath(path);

		FTPClient.RemoveFile(fullPath);

		wxString tempPath = wxFileName::CreateTempFileName("script");
		wxFile file(tempPath, wxFile::write);
		file.Write(wxString::FromUTF8(dataToSave));
		file.Close();

		if (!FTPClient.UploadFile(tempPath.ToStdString(), fullPath, true)) {
			wxMessageDialog fileNotSentDialog(this, "File Not Sent", "The file was not sent to the FTP server", wxOK | wxICON_ERROR);
		fileNotSentDialog.ShowModal();
		}

		wxRemoveFile(tempPath);
	} else {
		wxMessageDialog addressInvalidDialog(this, "This URL is invalid", "Invalid URL", wxOK | wxICON_ERROR);
		addressInvalidDialog.ShowModal();
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