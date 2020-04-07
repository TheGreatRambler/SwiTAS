#pragma once

#include <cstdio>
#include <cstdlib>
#include <ffms.h>
#include <ostream>
#include <rapidjson/document.h>
#include <vector>
#include <wx/process.h>
#include <wx/stdstream.h>
#include <wx/textctrl.h>
#include <wx/utils.h>
#include <wx/wx.h>

#include "../helpers.hpp"

// TODO: Use FFMS2 and Youtube-DL
// Requires Youtube-DL to be downloaded
class VideoComparisonViewer : public wxFrame {
private:
	wxBoxSizer* mainSizer;

	wxTextCtrl* urlInput;
	wxListBox* videoFormatsList;

	wxTextCtrl* consoleLog;

	std::string url;

	// Needs to be set by something, without trailing slash
	wxString projectDir;
	wxStreamToTextRedirector* redirectOutput;

	std::vector<int> formatsArray;

	void onVideoDownloaded(wxProcessEvent& event);

	void displayVideoFormats(wxCommandEvent& event);

	void onFormatSelection(wxCommandEvent& event);

public:
	VideoComparisonViewer();
};