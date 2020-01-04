#pragma once

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>

namespace Helpers {
	std::string getFilePath(std::string path) {
		// Just go back one folder :)
		return "../" + path;
	}

	void getGlobalSettings(rapidjson::Document* d) {
		std::ifstream settingsFile(getFilePath("mainSettings.json"));
		std::string content((std::istreambuf_iterator<char>(settingsFile)), (std::istreambuf_iterator<char>()));
		// Allow comments in JSON
		d->Parse<rapidjson::kParseCommentsFlag>(content.c_str());
	}
}