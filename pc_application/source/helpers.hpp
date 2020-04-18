#pragma once

#define _USE_MATH_DEFINES

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
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/wx.h>

namespace HELPERS {
	std::string resolvePath(std::string path);

	std::vector<std::string> splitString(const std::string s, char delim);
	std::string joinString(std::vector<std::string> strings, std::string delimiter);

	wxBitmapButton* getBitmapButton(wxWindow* parentFrame, rapidjson::Document* settings, const char* name);

	rapidjson::Document getSettingsFile(std::string filename);
	rapidjson::Document getSettingsFromString(std::string jsonString);

	float normalizeRadian(float angle);

	std::string exec(const char* cmd);

	wxBitmap* getBitmapFromJPEGData(std::vector<uint8_t> jpegBuffer);

	int char2int(char input);

	void hex2bin(const char* src, uint8_t* target);

	const uint16_t getHammingDistance(std::string hexString1, std::string hexString2);

	std::string makeRelative(std::string path, std::string rootDir);
	std::string makeFromRelative(std::string path, std::string rootDir);
}