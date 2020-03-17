#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include <numeric>
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

	wxBitmapButton* getBitmapButton(wxFrame* parentFrame, std::string path, int width, int height);

	float normalizeRadian(float angle);
}