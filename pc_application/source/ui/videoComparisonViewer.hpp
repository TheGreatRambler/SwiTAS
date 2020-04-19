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
#include <unordered_map>
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

	std::unordered_map<int, int> widthFromHeight{
		// https://support.google.com/youtube/answer/6375112?co=GENIE.Platform%3DDesktop&hl=en
		{ 2160, 3840 },
		{ 1440, 2560 },
		{ 1080, 1920 },
		{ 720, 1280 },
		{ 480, 854 },
		{ 360, 640 },
		{ 240, 426 },
		// https://en.wikipedia.org/wiki/Display_resolution
		{ 600, 800 },
		{ 768, 1024 },
		{ 800, 1280 },
		{ 1024, 1280 },
		{ 900, 1600 },
		{ 864, 1536 },
		{ 1152, 2048 },
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
	std::thread* indexingThread;
	moodycamel::ConcurrentQueue<std::string> indexingOutput;
	std::atomic_bool indexingDone = false;

	std::vector<std::string> formatsArray;
	std::vector<std::string> formatsMetadataArray;

	uint8_t videoExists     = false;
	uint8_t processingVideo = false;
	int recentVideoIndex    = -1;
	int selectedFormatIndex;

	char ffms2Errmsg[1024];
	FFMS_ErrorInfo ffms2Errinfo;

	FFMS_VideoSource* videosource          = NULL;
	const FFMS_VideoProperties* videoprops = NULL;
	FFMS_Index* videoIndex;
	FFMS_Indexer* videoIndexer;

	wxSize videoDimensions;
	DrawingCanvasBitmap* videoCanvas;
	wxSizerItem* videoCanvasSizerItem;

	void onCommandDone(wxProcessEvent& event);

	void printFfms2Error() {
		consoleLog->AppendText(wxString::Format("FFMS2 error: %s\n", wxString(ffms2Errinfo.Buffer)));
	}

	void indexVideo();
	void parseVideo();

	void indexingVideoThread();
	void finalizeIndexVideoThread();
	static int FFMS_CC onIndexingProgress(int64_t current, int64_t total, void* queue) {
		((moodycamel::ConcurrentQueue<std::string>*)queue)->enqueue(wxString::Format("%f percent indexed\n", ((double)current / total) * 100).ToStdString());
		return 0;
	}

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