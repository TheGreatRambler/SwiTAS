#pragma once

#define BUFSIZE 5120

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ffms.h>
#include <memory>
#include <ostream>
#include <picosha1.hpp>
#include <rapidjson/document.h>
#include <thread>
#include <vector>
#include <wx/longlong.h>
#include <wx/msgdlg.h>
#include <wx/process.h>
#include <wx/rawbmp.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stdstream.h>
#include <wx/textctrl.h>
#include <wx/url.h>
#include <wx/utils.h>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "../helpers.hpp"
#include "drawingCanvas.hpp"

struct VideoEntry {
	std::string videoUrl;
	std::string videoName;
	std::string videoFilename;
	std::string videoMetadata;
	std::string videoPath;
	std::string videoIndexerPath;
};

// TODO: Use FFMS2 and Youtube-DL
// Requires Youtube-DL to be downloaded
class VideoComparisonViewer : public wxFrame {
private:
	enum RUNNING_COMMAND : uint8_t {
		NO_COMMAND,
		DOWNLOAD_VIDEO,
	};

	rapidjson::Document* mainSettings;

	std::vector<std::shared_ptr<VideoEntry>>& videoEntries;

	wxBoxSizer* mainSizer;
	wxBoxSizer* inputSizer;

	wxSpinCtrl* frameSelect;
	wxSlider* frameSlider;

	// This CAN exceed the actual number of frames
	int currentFrame = 0;

	wxTextCtrl* urlInput;
	wxListBox* videoFormatsList;

	wxTextCtrl* consoleLog;

	std::string videoName;
	std::string videoFilename;
	std::string url;
	std::string fullVideoPath;
	std::string fullVideoIndexerPath;

	// Needs to be set by something, without trailing slash
	wxString projectDir;

	wxProcess* commandProcess;
	RUNNING_COMMAND currentRunningCommand = RUNNING_COMMAND::NO_COMMAND;
	std::shared_ptr<std::thread> indexingThread;
	moodycamel::ConcurrentQueue<std::string> indexingOutput;
	std::atomic_bool indexingDone;

	std::vector<int> formatsArray;
	std::vector<std::string> formatsMetadataArray;

	uint8_t videoExists  = false;
	int recentVideoIndex = -1;
	int selectedFormatIndex;
	char ffms2Errmsg[1024];
	FFMS_ErrorInfo ffms2Errinfo;
	FFMS_VideoSource* videosource          = NULL;
	const FFMS_VideoProperties* videoprops = NULL;
	FFMS_Index* videoIndex;

	wxSize videoDimensions;
	DrawingCanvasBitmap* videoCanvas;
	wxSizerItem* videoCanvasSizerItem;

	void onCommandDone(wxProcessEvent& event);

	// Hack to make c style function callbacks work
	static int FFMS_CC onIndexingProgress(int64_t current, int64_t total, void* self) {
		((VideoComparisonViewer*)self)->consoleLog->AppendText(wxString::Format("%f percent indexed\n", ((double)current / total) * 100));
		return 0;
	}

	void printFfms2Error() {
		consoleLog->AppendText(wxString::Format("FFMS2 error: %s\n", wxString(ffms2Errinfo.Buffer)));
	}

	void indexVideo();
	void parseVideo();

	void addToRecentVideoList();

	void drawFrame(int frame);

	void displayVideoFormats(wxCommandEvent& event);
	void onFormatSelection(wxCommandEvent& event);
	void frameChosenSpin(wxSpinEvent& event);
	void frameChosenSlider(wxCommandEvent& event);

	void onIdle(wxIdleEvent& event);
	void onClose(wxCloseEvent& event);

public:
	VideoComparisonViewer(rapidjson::Document* settings, std::vector<std::shared_ptr<VideoEntry>>& entries, wxString projectDirectory);

	// Called when running a frame from the list thing
	void seekRelative(int relativeFrame);

	void openWithRecent(int index);

	DECLARE_EVENT_TABLE();
};