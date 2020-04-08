#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ffms.h>
#include <ostream>
#include <rapidjson/document.h>
#include <vector>
#include <wx/process.h>
#include <wx/rawbmp.h>
#include <wx/stdstream.h>
#include <wx/textctrl.h>
#include <wx/url.h>
#include <wx/utils.h>
#include <wx/wx.h>

#include "../helpers.hpp"
#include "drawingCanvas.hpp"

// TODO: Use FFMS2 and Youtube-DL
// Requires Youtube-DL to be downloaded
class VideoComparisonViewer : public wxFrame {
private:
	wxBoxSizer* mainSizer;

	wxTextCtrl* urlInput;
	wxListBox* videoFormatsList;

	wxTextCtrl* consoleLog;

	std::string url;
	std::string fullVideoPath;

	// Needs to be set by something, without trailing slash
	wxString projectDir;
	wxStreamToTextRedirector* redirectOutput;

	std::vector<int> formatsArray;

	char ffms2Errmsg[1024];
	FFMS_ErrorInfo ffms2Errinfo;
	FFMS_VideoSource* videosource;
	const FFMS_VideoProperties* videoprops;

	wxSize videoDimensions;
	DrawingCanvasBitmap* videoCanvas;

	void onVideoDownloaded(wxProcessEvent& event);
	int FFMS_CC onIndexingProgress(int64_t current, int64_t total, void* unused) {
		consoleLog->AppendText(wxString::Format("%lu bytes of %lu bytes indexed\n", current, total));
	}
	void printFfms2Error() {
		consoleLog->AppendText(wxString::Format("FFMS2 error: %s\n", wxString(ffms2Errinfo.Buffer)));
	}

	void drawFrame(int frame);

	void displayVideoFormats(wxCommandEvent& event);

	void onFormatSelection(wxCommandEvent& event);

public:
	VideoComparisonViewer();
};