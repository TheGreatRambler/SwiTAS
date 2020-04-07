#include "videoComparisonViewer.hpp"

VideoComparisonViewer::VideoComparisonViewer() {

	mainSizer = new wxBoxSizer(wxVERTICAL);

	urlInput         = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_CENTRE);
	videoFormatsList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);

	consoleLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

	urlInput->Bind(wxEVT_TEXT_ENTER, &VideoComparisonViewer::displayVideoFormats, this);
	videoFormatsList->Bind(wxEVT_LISTBOX, &VideoComparisonViewer::onFormatSelection, this);

	Bind(wxEVT_END_PROCESS, &VideoComparisonViewer::onVideoDownloaded, this);

	mainSizer->Add(urlInput, 0, wxEXPAND | wxALL);
	mainSizer->Add(videoFormatsList, 0, wxEXPAND | wxALL);
	mainSizer->Add(consoleLog, 0, wxEXPAND | wxALL);

	// Hide by default
	videoFormatsList->Show(false);
	consoleLog->Show(true);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

void VideoComparisonViewer::displayVideoFormats(wxCommandEvent& event) {
	url = urlInput->GetLineText(0).ToStdString();
	formatsArray.clear();
	// Due to this https://forums.wxwidgets.org/viewtopic.php?t=20321
	videoFormatsList->Deselect(videoFormatsList->GetSelection());
	videoFormatsList->Clear();
	// First, get a temp file
	wxString tempJsonLocation = wxFileName::CreateTempFileName("json-data");
	// Dump data into file
	HELPERS::exec(("youtube-dl -j '" + url + "' > " + tempJsonLocation.ToStdString()).c_str());
	// Read data
	rapidjson::Document jsonData = HELPERS::getSettingsFile(tempJsonLocation.ToStdString());
	// TODO print info
	// For now, loop through formats
	int i = 0;
	for(auto const& format : jsonData["formats"].GetArray()) {
		// Check thing.json for an example
		std::string formatString = format["format"].GetString();
		int formatID             = strtol(format["format_id"].GetString(), nullptr, 10);

		if(format["format_note"].GetString() != "tiny") {
			// Tiny means it only supports audio, we want video
			int width           = format["width"].GetInt();
			int height          = format["height"].GetInt();
			int fps             = format["fps"].GetInt();
			wxString formatItem = wxString::Format("%dx%d, %d fps", width, height, fps);
			videoFormatsList->InsertItems(1, &formatItem, i);

			formatsArray[i] = formatID;
			i++;
		}
	}

	remove(tempJsonLocation.c_str());
	videoFormatsList->Show(true);
}

void VideoComparisonViewer::onFormatSelection(wxCommandEvent& event) {
	int selectedFormat = formatsArray[event.GetInt()];
	// Streaming download from youtube-dl
	wxProcess* downloadProcess = new wxProcess(this);
	downloadProcess->Redirect();
	wxStdInputStreamBuffer* consoleOutputStream = new wxStdInputStreamBuffer(*downloadProcess->GetInputStream());
	redirectOutput                              = new wxStreamToTextRedirector(consoleLog, new std::ostream(consoleOutputStream));

	// Show console
	consoleLog->Show(true);
	long pid = wxExecute(wxString::Format("youtube-dl -f %d -o %s/videos/%(title)s.%(ext)s '%s'", selectedFormat, projectDir, url), wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE, downloadProcess);
}

void VideoComparisonViewer::onVideoDownloaded(wxProcessEvent& event) {
	// Downloaded, so hide console
	delete redirectOutput;
	consoleLog->Show(false);
	// Video downloaded, open ffms2
	// The location is in youtube-dl output somewhere
}