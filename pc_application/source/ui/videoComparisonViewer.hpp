#pragma once

#define BUFSIZE 8192

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
	enum RUNNING_COMMAND : uint8_t {
        NO_COMMAND,
        DOWNLOAD_VIDEO,
    }

	wxBoxSizer* mainSizer;

	wxTextCtrl* urlInput;
	wxListBox* videoFormatsList;

	wxTextCtrl* consoleLog;

	std::string url;
	std::string fullVideoPath;

	// Needs to be set by something, without trailing slash
	wxString projectDir;

	wxProcess* commandProcess;
	RUNNING_COMMAND currentRunningCommand = RUNNING_COMMAND::NO_COMMAND;

	std::vector<int> formatsArray;

	char ffms2Errmsg[1024];
	FFMS_ErrorInfo ffms2Errinfo;
	FFMS_VideoSource* videosource;
	const FFMS_VideoProperties* videoprops;

	wxSize videoDimensions;
	DrawingCanvasBitmap* videoCanvas;

	void onCommandDone(wxProcessEvent& event);

	int FFMS_CC onIndexingProgress(int64_t current, int64_t total, void* unused) {
		consoleLog->AppendText(wxString::Format("%lu bytes of %lu bytes indexed\n", current, total));
	}
	void printFfms2Error() {
		consoleLog->AppendText(wxString::Format("FFMS2 error: %s\n", wxString(ffms2Errinfo.Buffer)));
	}

	void parseVideo();

	void drawFrame(int frame);

	void displayVideoFormats(wxCommandEvent& event);

	void onFormatSelection(wxCommandEvent& event);

	void onIdle(wxIdleEvent& event);

public:
	VideoComparisonViewer();

	DECLARE_EVENT_TABLE();
};