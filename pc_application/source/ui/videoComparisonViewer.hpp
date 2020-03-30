#pragma once

#include <rapidjson/document.h>
#include <wx/wx.h>

#include "../helpers.hpp"

// TODO: Use FFMS2 and Youtube-DL
// Requires Youtube-DL to be downloaded
class VideoComparisonViewer : public wxFrame {
private:
	void displayVideoFormats(std::string url);

public:
	VideoComparisonViewer();
};