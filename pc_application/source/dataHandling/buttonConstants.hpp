#pragma once

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

#include "../sharedNetworkCode/buttonData.hpp"

// So that types are somewhat unified
typedef uint32_t FrameNum;
typedef uint16_t SavestateBlockNum;
typedef uint16_t BranchNum;

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

struct Savestate {
	// I don't know what to put here yet
	uint32_t frame;
};

typedef std::vector<std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>>> SavestateHookBlock;
struct SavestateHook {
	std::string dHash;
	wxBitmap* screenshot;
	uint64_t runFinalTasDelayFrames;
	SavestateHookBlock inputs;
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