#include "videoComparisonViewer.hpp"

VideoComparisonViewer::VideoComparisonViewer(rapidjson::Document* settings) {
	mainSettings = settings;

	ffms2Errinfo.Buffer     = ffms2Errmsg;
	ffms2Errinfo.BufferSize = sizeof(ffms2Errmsg);
	ffms2Errinfo.ErrorType  = FFMS_ERROR_SUCCESS;
	ffms2Errinfo.SubType    = FFMS_ERROR_SUCCESS;

	mainSizer = new wxBoxSizer(wxVERTICAL);

	urlInput         = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_CENTRE);
	videoFormatsList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);

	inputSizer  = new wxBoxSizer(wxHORIZONTAL);
	frameSelect = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	frameSlider = new wxSlider(this, wxID_ANY, 0, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_LABELS | wxSL_HORIZONTAL);

	frameSelect->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &VideoComparisonViewer::frameChosenSpin, this);
	frameSlider->Bind(wxEVT_SLIDER, &VideoComparisonViewer::frameChosenSlider, this);

	inputSizer->Add(frameSelect, 1);
	inputSizer->Add(frameSlider, 1);

	consoleLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

	urlInput->Bind(wxEVT_TEXT_ENTER, &VideoComparisonViewer::displayVideoFormats, this);
	videoFormatsList->Bind(wxEVT_LISTBOX, &VideoComparisonViewer::onFormatSelection, this);
	Bind(wxEVT_END_PROCESS, &VideoComparisonViewer::onCommandDone, this);

	videoCanvas = new DrawingCanvasBitmap(this, wxDefaultSize);

	mainSizer->Add(urlInput, 0, wxEXPAND | wxALL);
	mainSizer->Add(videoFormatsList, 0, wxEXPAND | wxALL);
	mainSizer->Add(consoleLog, 0, wxEXPAND | wxALL);
	mainSizer->Add(videoCanvas, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(inputSizer, 1, wxEXPAND | wxALL);

	// Hide by default
	videoFormatsList->Show(false);
	consoleLog->Show(true);
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
END_EVENT_TABLE()
// clang-format on

void VideoComparisonViewer::onIdle(wxIdleEvent& event) {
	// Check command output
	if(currentRunningCommand != RUNNING_COMMAND::NO_COMMAND) {
		wxInputStream* inputStream = commandProcess->GetInputStream();
		if(inputStream->CanRead()) {
			consoleLog->Show(true);

			char buffer[BUFSIZE];
			std::size_t count = -1;
			wxString text;
			// https://forums.wxwidgets.org/viewtopic.php?t=24390#p104181
			while(!inputStream->Eof() && count != 0) {
				inputStream->Read(buffer, BUFSIZE - 1);
				count = inputStream->LastRead();
				if(count > 0) {
					text.Append(buffer, count);
				}
			}

			consoleLog->AppendText(text);
		}
	}
}

void VideoComparisonViewer::onCommandDone(wxProcessEvent& event) {
	if(currentRunningCommand == RUNNING_COMMAND::DOWNLOAD_VIDEO) {
		// Finished downloading video
		currentRunningCommand = RUNNING_COMMAND::NO_COMMAND;
		delete commandProcess;
		commandProcess = nullptr;

		parseVideo();
	}
}

void VideoComparisonViewer::displayVideoFormats(wxCommandEvent& event) {
	url = urlInput->GetLineText(0).ToStdString();
	wxURL urlData;
	if(urlData.SetURL(url) == wxURL_NOERR) {
		// All these commands will block, which could be a problem for bad internet connections
		std::string videoName = HELPERS::exec(("youtube-dl --get-filename -o '%(title)s.%(ext)s' " + url).c_str());
		// Replace spaces with underscores
		std::replace(videoName.begin(), videoName.end(), ' ', '_');
		fullVideoPath = projectDir.ToStdString() + "/videos/" + videoName;
		formatsArray.clear();
		// Due to this https://forums.wxwidgets.org/viewtopic.php?t=20321
		videoFormatsList->Deselect(videoFormatsList->GetSelection());
		videoFormatsList->Clear();
		// First, get a temp file
		wxString tempJsonLocation = wxFileName::CreateTempFileName("json-data");
		// Dump data into file
		// This will block, so hopefully the internet is good
		HELPERS::exec(("youtube-dl -j '" + url + "' > " + tempJsonLocation.ToStdString()).c_str());
		// Read data
		rapidjson::Document jsonData = HELPERS::getSettingsFile(tempJsonLocation.ToStdString());
		// For now, loop through formats
		int i = 0;
		for(auto const& format : jsonData["formats"].GetArray()) {
			// Check thing.json for an example
			if(strcmp(format["format_note"].GetString(), "tiny") != 0) {
				// Tiny means it only supports audio, we want video
				int formatID        = strtol(format["format_id"].GetString(), nullptr, 10);
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
	} else {
		// Not a valid URL
		wxMessageDialog urlInvalidDialog(this, "This URL is invalid", "Invalid URL", wxOK);
		urlInvalidDialog.ShowModal();
	}
}

void VideoComparisonViewer::onFormatSelection(wxCommandEvent& event) {
	int selectedFormat = formatsArray[event.GetInt()];
	// Streaming download from youtube-dl
	wxProcess* commandProcess = new wxProcess(this);
	commandProcess->Redirect();

	// This will run and the progress will be seen in console
	long pid = wxExecute(wxString::Format("youtube-dl -f %d -o %s '%s'", selectedFormat, wxString::FromUTF8(fullVideoPath), url), wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE, commandProcess);
}

void VideoComparisonViewer::parseVideo() {
	// https://github.com/FFMS/ffms2/blob/master/doc/ffms2-api.md
	FFMS_Indexer* videoIndexer = FFMS_CreateIndexer(fullVideoPath.c_str(), &ffms2Errinfo);
	if(videoIndexer == NULL) {
		printFfms2Error();
		return;
	}
	// This is helpful for cpp callbacks
	// https://stackoverflow.com/a/29817048/9329945
	FFMS_SetProgressCallback(videoIndexer, &VideoComparisonViewer::onIndexingProgress, NULL);

	FFMS_Index* index = FFMS_DoIndexing2(videoIndexer, FFMS_IEH_ABORT, &ffms2Errinfo);
	if(index == NULL) {
		printFfms2Error();
		return;
	}

	int trackno = FFMS_GetFirstTrackOfType(index, FFMS_TYPE_VIDEO, &ffms2Errinfo);
	if(trackno < 0) {
		printFfms2Error();
		return;
	}

	videosource = FFMS_CreateVideoSource(fullVideoPath.c_str(), trackno, index, 1, FFMS_SEEK_NORMAL, &ffms2Errinfo);
	if(videosource == NULL) {
		printFfms2Error();
		return;
	}

	FFMS_DestroyIndex(index);

	videoprops                  = FFMS_GetVideoProperties(videosource);
	const FFMS_Frame* propframe = FFMS_GetFrame(videosource, 0, &ffms2Errinfo);

	videoDimensions.SetWidth(propframe->EncodedWidth);
	videoDimensions.SetHeight(propframe->EncodedHeight);
	videoCanvas->SetSize(videoDimensions);

	int pixfmts[2];
	pixfmts[0] = FFMS_GetPixFmt("rgb24");
	pixfmts[1] = -1;

	if(FFMS_SetOutputFormatV2(videosource, pixfmts, propframe->EncodedWidth, propframe->EncodedHeight, FFMS_RESIZER_BICUBIC, &ffms2Errinfo)) {
		printFfms2Error();
		return;
	}

	videoExists = true;
	consoleLog->Show(false);
	frameSelect->SetRange(0, videoprops->NumFrames);
	frameSlider->SetRange(0, videoprops->NumFrames);

	frameSelect->SetValue(0);
	frameSlider->SetValue(0);

	drawFrame(0);
}

void VideoComparisonViewer::drawFrame(int frame) {
	currentFrame = frame;

	consoleLog->Show(false);
	const FFMS_Frame* curframe = FFMS_GetFrame(videosource, frame, &ffms2Errinfo);
	if(curframe == NULL) {
		printFfms2Error();
		return;
	}

	// In the RGB format, the data is always in the first plane
	uint8_t* data = curframe->Data[0];

	wxBitmap* videoFrame = new wxBitmap(videoDimensions, 24);
	wxNativePixelData nativePixelData(*videoFrame);

	wxNativePixelData::Iterator p(nativePixelData);

	for(int pixelIndex = 0; pixelIndex < (videoDimensions.GetWidth() * videoDimensions.GetHeight()); pixelIndex++) {
		int dataIndex = pixelIndex * 3;

		p.Red()   = data[dataIndex];
		p.Green() = data[dataIndex + 1];
		p.Blue()  = data[dataIndex + 2];

		++p;
	}

	// The canvas will consume the bitmap
	videoCanvas->setBitmap(videoFrame);

	videoCanvas->Show(true);
	// TODO run FFMS_DestroyVideoSource(videosource); at the end of something
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
		}
	}
}