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

#include "../../sharedNetworkCode/buttonData.hpp"
#include "../helpers.hpp"
#include "dataProcessing.hpp"

// So that types are somewhat unified
typedef uint32_t FrameNum;

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

	static constexpr int32_t axisMin = -30000;
	static constexpr int32_t axisMax = 30000;

	// Struct containing button info
	struct ButtonInfo {
		std::string scriptName;
		std::string normalName;
		std::string viewName;
		wxImage* onIcon;
		wxImage* offIcon;
		wxBitmap* onBitmapIcon;
		wxBitmap* offBitmapIcon;
		// Resized images for the UI
		wxBitmap* resizedListOnBitmap;
		wxBitmap* resizedListOffBitmap;
		wxBitmap* resizedGridOnBitmap;
		wxBitmap* resizedGridOffBitmap;
		// Keybinding
		wxChar toggleKeybind;
		// Grid stuff
		uint8_t gridX;
		uint8_t gridY;
	};

	struct SavestateHook {
		// I don't know what to put here yet
		uint32_t frame;
	};

	enum ControllerNumberValues : uint8_t {
		LEFT_X,
		LEFT_Y,
		RIGHT_X,
		RIGHT_Y,
		ACCEL_X,
		ACCEL_Y,
		ACCEL_Z,
		GYRO_1,
		GYRO_2,
		GYRO_3,
	};

	const uint8_t KeyWidth  = 11;
	const uint8_t KeyHeight = 4;

	// TODO finish these
	// https://gitlab.gnome.org/GNOME/gtk/blob/master/gdk/gdkkeysyms.h
	std::map<Btn, std::shared_ptr<ButtonInfo>> buttonMapping;

	void setupButtonMapping(rapidjson::Document* mainSettings);

	void textToFrames(DataProcessing* dataProcessing, std::string text, FrameNum startLoc, bool insertPaste, bool placePaste);
	std::string framesToText(DataProcessing* dataProcessing, FrameNum startLoc, FrameNum endLoc);

	void transferControllerData(std::shared_ptr<ControllerData> src, std::shared_ptr<ControllerData> dest, bool placePaste);

	bool isEmptyControllerData(std::shared_ptr<ControllerData> data);
};

// Some good typedef's

typedef ButtonData::ButtonInfo ButtonInfo;
typedef ButtonData::SavestateHook SavestateHook;
typedef ButtonData::ControllerNumberValues ControllerNumberValues;