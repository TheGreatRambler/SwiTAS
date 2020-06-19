#include "videoComparisonViewer.hpp"

VideoComparisonViewer::VideoComparisonViewer(wxFrame* parent, std::function<void(VideoComparisonViewer*)> callback, rapidjson::Document* settings, std::vector<std::shared_ptr<VideoEntry>>& entries, wxString projectDirectory)
	: wxFrame(parent, wxID_ANY, "Video Comparison Viewer", wxDefaultPosition, wxSize(600, 400), wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT)
	, videoEntries(entries) {
	// https://www.youtube.com/watch?v=su61pXgmJcw
	mainSettings  = settings;
	projectDir    = projectDirectory;
	closeCallback = callback;

	ffms2Errinfo.Buffer     = ffms2Errmsg;
	ffms2Errinfo.BufferSize = sizeof(ffms2Errmsg);
	ffms2Errinfo.ErrorType  = FFMS_ERROR_SUCCESS;
	ffms2Errinfo.SubType    = FFMS_ERROR_SUCCESS;

	mainSizer = new wxBoxSizer(wxVERTICAL);
	urlSizer  = new wxBoxSizer(wxHORIZONTAL);

	urlInput         = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_CENTRE);
	trashVideo       = HELPERS::getSystemBitmapButton(this, wxART_DELETE);
	videoFormatsList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);

	inputSizer  = new wxBoxSizer(wxHORIZONTAL);
	frameSelect = new wxSpinCtrl(this, wxID_ANY, "Select Frame", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	frameSlider = new wxSlider(this, wxID_ANY, 0, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_LABELS | wxSL_HORIZONTAL);

	frameSelect->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &VideoComparisonViewer::frameChosenSpin, this);
	frameSlider->Bind(wxEVT_SLIDER, &VideoComparisonViewer::frameChosenSlider, this);

	inputSizer->Add(frameSelect, 1);
	inputSizer->Add(frameSlider, 1);

	consoleLog  = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	videoCanvas = new DrawingCanvasBitmap(this, wxSize(1, 1));

	urlInput->Bind(wxEVT_TEXT_ENTER, &VideoComparisonViewer::displayVideoFormats, this);
	trashVideo->Bind(wxEVT_BUTTON, &VideoComparisonViewer::onTrashVideo, this);
	urlInput->Bind(wxEVT_ENTER_WINDOW, &VideoComparisonViewer::onEnterUrl, this);
	videoCanvas->Bind(wxEVT_ENTER_WINDOW, &VideoComparisonViewer::onEnterVideo, this);
	videoFormatsList->Bind(wxEVT_LISTBOX, &VideoComparisonViewer::onFormatSelection, this);
	Bind(wxEVT_END_PROCESS, &VideoComparisonViewer::onCommandDone, this);

	urlSizer->Add(urlInput, 1, wxEXPAND | wxALL);
	urlSizer->Add(trashVideo, 0);

	mainSizer->Add(urlSizer, 0, wxEXPAND | wxALL);
	mainSizer->Add(videoFormatsList, 2, wxEXPAND | wxALL);
	mainSizer->Add(consoleLog, 3, wxEXPAND | wxALL);
	videoCanvasSizerItem = mainSizer->Add(videoCanvas, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(inputSizer, 0, wxEXPAND | wxALL);

	// Hide by default
	trashVideo->Show(false);
	videoFormatsList->Show(false);
	videoCanvas->Show(false);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

// clang-format off
BEGIN_EVENT_TABLE(VideoComparisonViewer, wxFrame)
	EVT_IDLE(VideoComparisonViewer::onIdle)
	EVT_CLOSE(VideoComparisonViewer::onClose)
END_EVENT_TABLE()
// clang-format on

void VideoComparisonViewer::onIdle(wxIdleEvent& event) {
	// Check command output
	if(currentRunningCommand != RUNNING_COMMAND::NO_COMMAND) {
		wxInputStream* inputStream = commandProcess->GetInputStream();
		if(inputStream->CanRead()) {
			consoleLog->Show(true);
			Layout();

			// https://forums.wxwidgets.org/viewtopic.php?t=24390#p104181

			char buffer[BUFSIZE];
			wxString text;
			inputStream->Read(buffer, BUFSIZE - 1);
			std::size_t count = inputStream->LastRead();
			if(count > 0) {
				text.Append(buffer, count);
			}

			consoleLog->AppendText(text);
		}
	}
}

void VideoComparisonViewer::onClose(wxCloseEvent& event) {
	FFMS_DestroyVideoSource(videosource);

	closeCallback(this);

	Destroy();
}

void VideoComparisonViewer::onTrashVideo(wxCommandEvent& event) {
	const char* videoPath        = videoEntries[recentVideoIndex]->videoPath.c_str();
	const char* videoIndexerPath = videoEntries[recentVideoIndex]->videoIndexerPath.c_str();
	remove(videoPath);
	remove(videoIndexerPath);
	videoEntries.erase(videoEntries.begin() + recentVideoIndex);

	Close(true);
}

void VideoComparisonViewer::onCommandDone(wxProcessEvent& event) {
	if(currentRunningCommand == RUNNING_COMMAND::DOWNLOAD_VIDEO) {
		// Finished downloading video
		currentRunningCommand = RUNNING_COMMAND::NO_COMMAND;
		delete commandProcess;
		commandProcess = nullptr;

		indexVideo();
	}
}

void VideoComparisonViewer::displayVideoFormats(wxCommandEvent& event) {
	url = urlInput->GetLineText(0).ToStdString();

	trashVideo->Show(false);
	Layout();

	// All these commands will block, which could be a problem for bad internet connections
	std::string videoCheck = HELPERS::exec(("youtube-dl --get-filename \"" + url + "\"").c_str());
	if(videoCheck.find("is not recognized") != std::string::npos || videoCheck.find("command not found") != std::string::npos) {
		wxMessageDialog errorDialog(this, "Youtube-DL Not Found", "The Youtube-DL executable was not found", wxOK | wxICON_ERROR);
		errorDialog.ShowModal();
	} else if(videoCheck.size() != 0) {
		formatsArray.clear();
		formatsMetadataArray.clear();
		// Due to this https://forums.wxwidgets.org/viewtopic.php?t=20321
		videoFormatsList->Deselect(videoFormatsList->GetSelection());
		videoFormatsList->Clear();
		// This will block, so hopefully the internet is good
		std::string commandString = "youtube-dl -j \"" + url + "\"";
		std::string jsonString    = HELPERS::exec(commandString.c_str());
		// Read data
		rapidjson::Document jsonData = HELPERS::getSettingsFromString(jsonString);

		videoName     = std::string(jsonData["title"].GetString());
		videoFilename = picosha2::hash256_hex_string(std::string(jsonData["title"].GetString())) + std::string(".") + std::string(jsonData["ext"].GetString());

		// For now, loop through formats
		int i = 0;
		if(jsonData.IsObject()) {
			for(auto const& format : jsonData["formats"].GetArray()) {
				// Check thing.json for an example
				if(format.HasMember("height") && format["height"].IsInt()) {
					// For some stupid reason, twitch removes width
					std::string formatID = std::string(format["format_id"].GetString());
					int height           = format["height"].GetInt();

					int width;
					if(format.HasMember("width") && format["width"].IsInt()) {
						width = format["width"].GetInt();
					} else if(widthFromHeight.count(height)) {
						width = widthFromHeight[height];
					} else {
						// Oh heck, bail
						continue;
					}

					// Assume fps is 60 if it is not present
					int fps                    = (format.HasMember("fps") && format["fps"].IsInt()) ? format["fps"].GetInt() : 60;
					wxString formatItem        = wxString::Format("%dx%d, %d fps", width, height, fps);
					wxString compactFormatItem = wxString::Format("%dx%d-%dfps", width, height, fps);
					videoFormatsList->InsertItems(1, &formatItem, i);

					formatsArray.push_back(formatID);
					formatsMetadataArray.push_back(compactFormatItem.ToStdString());
					i++;
				}
			}
		} else {
			// Also not a valid URL
			wxMessageDialog urlInvalidDialog(this, "This URL is invalid", "Invalid URL", wxOK | wxICON_ERROR);
			urlInvalidDialog.ShowModal();
			return;
		}

		videoFormatsList->Show(true);
		Layout();
	} else {
		// Not a valid URL
		wxMessageDialog urlInvalidDialog(this, "This URL is invalid", "Invalid URL", wxOK | wxICON_ERROR);
		urlInvalidDialog.ShowModal();
	}
}

void VideoComparisonViewer::onFormatSelection(wxCommandEvent& event) {
	if(!processingVideo) {
		processingVideo     = true;
		selectedFormatIndex = event.GetInt();

		if(videoExists) {
			FFMS_DestroyVideoSource(videosource);
			videoExists = false;
		}

		consoleLog->Clear();

		std::string selectedFormat = formatsArray[selectedFormatIndex];
		std::string formatMetadata = formatsMetadataArray[selectedFormatIndex];

		rapidjson::GenericArray<false, rapidjson::Value> recentProjectsArray = (*mainSettings)["recentVideos"].GetArray();

		recentVideoIndex = -1;
		for(std::size_t i = 0; i < videoEntries.size(); i++) {
			if(videoEntries[i]->videoMetadata == formatMetadata && videoEntries[i]->videoName == videoName) {
				recentVideoIndex = i;
				break;
			}
		}

		if(recentVideoIndex == -1) {
			// New video, add the stuff
			consoleLog->AppendText("New video, have to download\n");

			fullVideoPath        = projectDir.ToStdString() + "/videos/" + formatsMetadataArray[selectedFormatIndex] + "-" + videoFilename;
			fullVideoIndexerPath = projectDir.ToStdString() + "/videos/" + formatsMetadataArray[selectedFormatIndex] + "-Indexer-" + videoFilename + ".bin";

			// Streaming download from youtube-dl
			commandProcess = new wxProcess(this);
			commandProcess->Redirect();

			wxString commandString = wxString::Format("youtube-dl -f %s -o %s %s", wxString::FromUTF8(selectedFormat), wxString::FromUTF8(fullVideoPath), url);

			// This will run and the progress will be seen in console
			currentRunningCommand = RUNNING_COMMAND::DOWNLOAD_VIDEO;
			wxExecute(commandString, wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE, commandProcess);
		} else {
			// Old video, load from disk
			consoleLog->AppendText("Old video, loading from disk\n");

			fullVideoPath        = videoEntries[recentVideoIndex]->videoPath;
			fullVideoIndexerPath = videoEntries[recentVideoIndex]->videoIndexerPath;
			indexVideo();
		}
	}
}

// Called when the video is selected from the menu
void VideoComparisonViewer::openWithRecent(int index) {
	consoleLog->AppendText("Old video, loading from disk\n");

	recentVideoIndex = index;

	urlInput->Clear();
	urlInput->SetValue(wxString::FromUTF8(videoEntries[recentVideoIndex]->videoUrl));

	fullVideoPath        = videoEntries[recentVideoIndex]->videoPath;
	fullVideoIndexerPath = videoEntries[recentVideoIndex]->videoIndexerPath;

	wxCommandEvent fakeCommandEvent;
	displayVideoFormats(fakeCommandEvent);
	videoFormatsList->Show(false);
	Layout();

	indexVideo();
}

void VideoComparisonViewer::indexVideo() {
	if(recentVideoIndex == -1) {
		consoleLog->AppendText("Start indexing video\n");
		// https://github.com/FFMS/ffms2/blob/master/doc/ffms2-api.md
		videoIndexer = FFMS_CreateIndexer(fullVideoPath.c_str(), &ffms2Errinfo);
		if(videoIndexer == NULL) {
			printFfms2Error();
			return;
		}

		videoIndex = FFMS_DoIndexing2(videoIndexer, FFMS_IEH_ABORT, &ffms2Errinfo);
		if(videoIndex == NULL) {
			printFfms2Error();
			return;
		}

		int res = FFMS_WriteIndex(fullVideoIndexerPath.c_str(), videoIndex, &ffms2Errinfo);
		if(res != 0) {
			printFfms2Error();
			return;
		}
		consoleLog->AppendText("Finish indexing video\n");

		// Finally, add to recent videos list and set the index to an actual one
		addToRecentVideoList();
		parseVideo();
	} else {
		// Read index from disk
		videoIndex = FFMS_ReadIndex(fullVideoIndexerPath.c_str(), &ffms2Errinfo);
		if(videoIndex == NULL) {
			printFfms2Error();
			return;
		}
		parseVideo();
	}
}

void VideoComparisonViewer::parseVideo() {
	int trackno = FFMS_GetFirstTrackOfType(videoIndex, FFMS_TYPE_VIDEO, &ffms2Errinfo);
	if(trackno < 0) {
		printFfms2Error();
		return;
	}

	videosource = FFMS_CreateVideoSource(fullVideoPath.c_str(), trackno, videoIndex, 1, FFMS_SEEK_NORMAL, &ffms2Errinfo);
	if(videosource == NULL) {
		printFfms2Error();
		return;
	}

	FFMS_DestroyIndex(videoIndex);

	videoprops                  = FFMS_GetVideoProperties(videosource);
	const FFMS_Frame* propframe = FFMS_GetFrame(videosource, 0, &ffms2Errinfo);

	videoDimensions.SetWidth(propframe->EncodedWidth);
	videoDimensions.SetHeight(propframe->EncodedHeight);
	videoCanvasSizerItem->SetRatio(videoDimensions);

	int pixfmts[2];
	pixfmts[0] = FFMS_GetPixFmt("rgb24");
	pixfmts[1] = -1;

	if(FFMS_SetOutputFormatV2(videosource, pixfmts, propframe->EncodedWidth, propframe->EncodedHeight, FFMS_RESIZER_BICUBIC, &ffms2Errinfo)) {
		printFfms2Error();
		return;
	}

	videoExists = true;
	consoleLog->Show(false);
	Layout();

	frameSelect->SetRange(0, videoprops->NumFrames);
	frameSlider->SetRange(0, videoprops->NumFrames);

	frameSelect->SetValue(0);
	frameSlider->SetValue(0);

	// Finally, we can let the user change videos if desired
	processingVideo = false;

	// Let them delete the video if desired
	trashVideo->Show(true);
	Layout();

	drawFrame(0);
}

void VideoComparisonViewer::addToRecentVideoList() {
	std::shared_ptr<VideoEntry> videoEntry = std::make_shared<VideoEntry>();

	videoEntry->videoUrl         = urlInput->GetLineText(0).ToStdString();
	videoEntry->videoName        = videoName;
	videoEntry->videoFilename    = videoFilename;
	videoEntry->videoMetadata    = formatsMetadataArray[selectedFormatIndex];
	videoEntry->videoPath        = fullVideoPath;
	videoEntry->videoIndexerPath = fullVideoIndexerPath;

	recentVideoIndex = videoEntries.size();
	videoEntries.push_back(videoEntry);
}

void VideoComparisonViewer::drawFrame(int frame) {
	// The frames in this case are video dependent, TODO add stuff for 60fps always
	currentFrame = frame;

	consoleLog->Show(false);
	const FFMS_Frame* curframe = FFMS_GetFrame(videosource, frame, &ffms2Errinfo);
	if(curframe == NULL) {
		printFfms2Error();
		return;
	}

	// In the RGB format, the data is always in the first plane
	uint8_t* data = curframe->Data[0];
	int linesize  = curframe->Linesize[0];

	wxBitmap* videoFrame = new wxBitmap(videoDimensions, 24);
	wxNativePixelData nativePixelData(*videoFrame);

	wxNativePixelData::Iterator p(nativePixelData);

	for(int y = 0; y < videoDimensions.GetHeight(); y++) {
		wxNativePixelData::Iterator rowStart = p;

		for(int x = 0; x < videoDimensions.GetWidth(); x++) {
			int start = y * linesize + x * 3;
			p.Red()   = data[start];
			p.Green() = data[start + 1];
			p.Blue()  = data[start + 2];

			++p;
		}

		p = rowStart;
		p.OffsetY(nativePixelData, 1);
	}

	// The canvas will consume the bitmap
	videoCanvas->setBitmap(videoFrame);

	videoCanvas->Show(true);
	Layout();
}

void VideoComparisonViewer::frameChosenSpin(wxSpinEvent& event) {
	int value = frameSelect->GetValue();
	if(videoExists) {
		drawFrame(value);
	}
	frameSlider->SetValue(value);
}
void VideoComparisonViewer::frameChosenSlider(wxCommandEvent& event) {
	int value = frameSlider->GetValue();
	if(videoExists) {
		drawFrame(value);
	}
	frameSelect->SetValue(value);
}

void VideoComparisonViewer::seekRelative(int relativeFrame) {
	// This is given the change that has passed, not the actual frame
	// Passed by DataProcessing
	if(videoExists) {
		currentFrame += relativeFrame;

		if(currentFrame > -1 && currentFrame < videoprops->NumFrames) {
			drawFrame(currentFrame);
			frameSlider->SetValue(currentFrame);
			frameSelect->SetValue(currentFrame);
		}
	}
}