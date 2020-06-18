#pragma once

#define SET_BIT(number, bit, loc) (number) ^= (-(unsigned long)(bit) ^ (number)) & (1UL << (loc))
#define GET_BIT(number, loc) ((number) >> (loc)) & 1U

#include <bitset>
#include <cstdio>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <wx/wx.h>

#include "../helpers.hpp"
#include "../sharedNetworkCode/buttonData.hpp"
#include "buttonConstants.hpp"
#include "dataProcessing.hpp"

// Forward declare to allow headers to include each other
class DataProcessing;

class ButtonData {
private:
	// Helper function to add transparency masks
	void maskifyBitmap(wxBitmap* bitmap, wxColour maskColor);

public:
	// Some of this is defined in network code

	// To convert names
	std::map<std::string, Btn> stringToButton {
		{ "A", Btn::A },
		{ "B", Btn::B },
		{ "X", Btn::X },
		{ "Y", Btn::Y },
		{ "L", Btn::L },
		{ "R", Btn::R },
		{ "ZL", Btn::ZL },
		{ "ZR", Btn::ZR },
		{ "SL", Btn::SL },
		{ "SR", Btn::SR },
		{ "DUP", Btn::DUP },
		{ "DDOWN", Btn::DDOWN },
		{ "DLEFT", Btn::DLEFT },
		{ "DRIGHT", Btn::DRIGHT },
		{ "PLUS", Btn::PLUS },
		{ "MINUS", Btn::MINUS },
		{ "HOME", Btn::HOME },
		{ "CAPT", Btn::CAPT },
		{ "LS", Btn::LS },
		{ "RS", Btn::RS },
	};

	// To convert to Btn
	std::unordered_map<std::string, Btn> scriptNameToButton;

	static constexpr int16_t axisMin = -30000;
	static constexpr int16_t axisMax = 30000;

	static constexpr uint8_t KeyWidth  = 11;
	static constexpr uint8_t KeyHeight = 4;

	std::map<Btn, std::shared_ptr<ButtonInfo>> buttonMapping;

	void setupButtonMapping(rapidjson::Document* mainSettings);

	FrameNum textToFrames(DataProcessing* dataProcessing, std::string text, FrameNum startLoc, bool insertPaste, bool placePaste);
	std::string framesToText(DataProcessing* dataProcessing, FrameNum startLoc, FrameNum endLoc, int playerIndex);

	void transferControllerData(ControllerData src, std::shared_ptr<ControllerData> dest, bool placePaste);

	bool isEmptyControllerData(std::shared_ptr<ControllerData> data);
};