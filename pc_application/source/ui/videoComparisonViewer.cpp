#include "videoComparisonViewer.hpp"

VideoComparisonViewer::VideoComparisonViewer() {}

void VideoComparisonViewer::displayVideoFormats(std::string url) {
	// First, get a temp file
	wxString tempJsonLocation = wxFileName::CreateTempFileName("json-data");
	// Dump data into file
	HELPERS::exec(("youtube-dl -j '" + url + "' > " + tempJsonLocation.ToStdString()).c_str());
	// Read data
	rapidjson::Document jsonData = HELPERS::getSettingsFile(tempJsonLocation.ToStdString());
	// TODO print info
	// For now, loop through formats
	for(auto const& format : jsonData["formats"].GetArray()) {
		// Check thing.json for an example
		std::string formatString = format["format"].GetString();
		int formatID             = format["format_id"].GetInt();
		// If data contains `audio only`, ignore it
	}
}