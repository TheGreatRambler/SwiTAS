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
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/wx.h>

namespace HELPERS {
	std::string resolvePath(std::string path);

	std::vector<std::string> splitString(const std::string s, char delim);
	std::string joinString(std::vector<std::string> strings, std::string delimiter);

	wxBitmapButton* getBitmapButton(wxWindow* parentFrame, rapidjson::Document* settings, const char* name);

	rapidjson::Document getSettingsFile(std::string filename);

	float normalizeRadian(float angle);

	std::string exec(const char* cmd);

	wxBitmap* getBitmapFromJPEGData(std::vector<uint8_t> jpegBuffer);

	int char2int(char input) {
		if(input >= '0' && input <= '9')
			return input - '0';
		if(input >= 'A' && input <= 'F')
			return input - 'A' + 10;
		if(input >= 'a' && input <= 'f')
			return input - 'a' + 10;
	}

	void hex2bin(const char* src, uint8_t* target) {
		// https://stackoverflow.com/a/17261928/9329945
		while(*src && src[1]) {
			*(target++) = char2int(*src) * 16 + char2int(src[1]);
			src += 2;
		}
	}

	const uint16_t getHammingDistance(std::string hexString1, std::string hexString2) {
		// Both hex strings MUST have the same size
		uint8_t m1[hexString1.length() / 2];
		uint8_t m2[hexString2.length() / 2];
		std::size_t size = hexString1.length() / 2;

		hex2bin(hexString1.c_str(), m1);
		hex2bin(hexString2.c_str(), m2);

		// https://gist.github.com/Miguellissimo/2faa7e3c3e1800a6bf97
		uint16_t counter = 0;

		for(std::size_t i = 0; i != size; ++i) {
			uint8_t diff = m1[i] ^ m2[i];

			diff = (diff & (uint8_t)0x55) + ((diff >> 1) & (uint8_t)0x55);
			diff = (diff & (uint8_t)0x33) + ((diff >> 2) & (uint8_t)0x33);
			diff = (diff & (uint8_t)0x0f) + ((diff >> 4) & (uint8_t)0x0f);

			counter += diff;
		}

		return counter;
	}
}