#pragma once

#define _USE_MATH_DEFINES

#ifdef _WIN32
#include <Windows.h>
#include <uxtheme.h>
#endif
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <rapidjson/document.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>
#include <wx/arrstr.h>
#include <wx/artprov.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>

namespace HELPERS {
	std::string resolvePath(std::string path);

	std::vector<std::string> splitString(const std::string s, char delim);
	std::string joinString(std::vector<std::string> strings, std::string delimiter);

	wxBitmapButton* getBitmapButton(wxWindow* parentFrame, rapidjson::Document* settings, const char* name);
	wxBitmapButton* getSystemBitmapButton(wxWindow* parentFrame, wxArtID id);

	void addDarkmodeWindows(wxWindow* window);

	rapidjson::Document getSettingsFile(std::string filename);
	rapidjson::Document getSettingsFromString(std::string jsonString);

	wxFileName getMainSettingsPath();

	float normalizeRadian(float angle);

	std::string exec(const char* cmd);

	wxImage getImageFromJPEGData(std::vector<uint8_t> jpegBuffer);
	wxString calculateDhash(wxImage image, int dhashWidth, int dhashHeight);

	const int getHammingDistance(wxString string1, wxString string2);

	wxBitmap* getDefaultSavestateScreenshot();

	std::string makeRelative(std::string path, std::string rootDir);
	std::string makeFromRelative(std::string path, std::string rootDir);
}