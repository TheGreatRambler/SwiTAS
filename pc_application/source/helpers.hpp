#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include <fstream>
#include <numeric>
#include <rapidjson/document.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <wx/filename.h>
#include <wx/wx.h>

namespace HELPERS {
	std::string resolvePath(std::string path);

	std::vector<std::string> splitString(const std::string s, char delim);
	std::string joinString(std::vector<std::string> strings, std::string delimiter);

	wxBitmapButton* getBitmapButton(wxWindow* parentFrame, rapidjson::Document* settings, const char* name);

	rapidjson::Document getSettingsFile(std::string filename);

	float normalizeRadian(float angle);
}