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
	wxFTP ftp;
	ftpAddress = ftpEntry->GetLineText(0).ToStdString();

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

	if(sscanf(ftpAddress.c_str(), "ftp://%[^:]:%[^@]@%[^:]:%d/%s", user, password, host, &port, path) == 5) {
		userSet     = true;
		passwordSet = true;
		portSet     = true;

		successful = true;
	} else {
		if(sscanf(ftpAddress.c_str(), "ftp://%[^:]:%d/%s", host, &port, path) == 3) {
			portSet = true;

			successful = true;
		} else {
			if(sscanf(ftpAddress.c_str(), "ftp://%[^/]/%s", host, path) == 2) {
				successful = true;
			}
		}
	}

	if(successful) {
		if(userSet)
			ftp.SetUser(wxString::FromUTF8(user));
		if(passwordSet)
			ftp.SetPassword(wxString::FromUTF8(password));

		uint8_t ftpConnectionSuccessful;
		if(portSet) {
			ftpConnectionSuccessful = ftp.Connect(wxString::FromUTF8(host), port);
		} else {
			ftpConnectionSuccessful = ftp.Connect(wxString::FromUTF8(host));
		}

		if(ftpConnectionSuccessful) {
			std::vector<std::string> pathParts = HELPERS::splitString(std::string(path), '/');

			ftp.ChDir("/");
			ftp.SetAscii();

			for(int i = 0; i < pathParts.size() - 1; i++) {
				std::string currentPath = "/";
				for(int j = 0; j < i; j++) {
					currentPath += (pathParts[j] + "/");
				}

				if(!ftp.ChDir(wxString::FromUTF8(currentPath))) {
					// Create the directory
					if(!ftp.MkDir(wxString::FromUTF8(pathParts[i]))) {
						wxMessageDialog ftpDirectoryError(this, "Could not create directory", "Invalid Directory", wxOK | wxICON_ERROR);
						ftpDirectoryError.ShowModal();

						ftp.Close();

						return;
					}
				}
			}

			wxString filename = wxString::FromUTF8(pathParts[pathParts.size() - 1]);

			if(ftp.FileExists(filename)) {
				// Do twice in case of firewall issues
				if(!ftp.RmFile(filename)) {
					ftp.RmFile(filename);
				}
			}

			// ftp.SetPassive(false);
			wxOutputStream* out = ftp.GetOutputStream(filename);

			// Just in case of firewall issues, try one more time
			if(!out) {
				out = ftp.GetOutputStream(filename);
			}

			if(out) {
				wxStringInputStream stringStream(wxString::FromUTF8(dataToSave));
				out->Write(stringStream);
				delete out;

				// To allow quick exporting later
				projectHandler->setLastEnteredFtpPath(ftpAddress);

				EndModal(wxID_OK);
			} else {
				wxMessageDialog fileErrorDialog(this, "Error when writing file", "File Writing Error", wxOK | wxICON_ERROR);
				fileErrorDialog.ShowModal();
			}

		} else {
			wxMessageDialog hostErrorDialog(this, wxString::Format("Could not log into %s", wxString::FromUTF8(ftpAddress)), "Invalid Host Or Port", wxOK | wxICON_ERROR);
			hostErrorDialog.ShowModal();
		}
	} else {
		wxMessageDialog addressInvalidDialog(this, "This URL is invalid", "Invalid URL", wxOK | wxICON_ERROR);
		addressInvalidDialog.ShowModal();
	}

	ftp.Close();
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