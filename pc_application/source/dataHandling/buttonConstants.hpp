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

#include "../../../sysmodule_application/saltynx_plugin/source/sdkTypes.hpp"
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

struct KeyboardKeyInfo {
	std::string scriptName;
};

struct KeyboardModifierKeyInfo {
	std::string scriptName;
};

struct MouseButtonInfo {
	std::string scriptName;
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
	ACCEL_X_LEFT,
	ACCEL_Y_LEFT,
	ACCEL_Z_LEFT,
	GYRO_X_LEFT,
	GYRO_Y_LEFT,
	GYRO_Z_LEFT,
	ANGLE_X_LEFT,
	ANGLE_Y_LEFT,
	ANGLE_Z_LEFT,
	ACCEL_X_RIGHT,
	ACCEL_Y_RIGHT,
	ACCEL_Z_RIGHT,
	GYRO_X_RIGHT,
	GYRO_Y_RIGHT,
	GYRO_Z_RIGHT,
	ANGLE_X_RIGHT,
	ANGLE_Y_RIGHT,
	ANGLE_Z_RIGHT,
DIRECTION_XX_LEFT,
DIRECTION_XY_LEFT,
DIRECTION_XZ_LEFT,
DIRECTION_YX_LEFT,
DIRECTION_YY_LEFT,
DIRECTION_YZ_LEFT,
DIRECTION_ZX_LEFT,
DIRECTION_ZY_LEFT,
DIRECTION_ZZ_LEFT,
DIRECTION_XX_RIGHT,
DIRECTION_XY_RIGHT,
DIRECTION_XZ_RIGHT,
DIRECTION_YX_RIGHT,
DIRECTION_YY_RIGHT,
DIRECTION_YZ_RIGHT,
DIRECTION_ZX_RIGHT,
DIRECTION_ZY_RIGHT,
DIRECTION_ZZ_RIGHT,
};

enum ExtraValues : uint8_t {
	TOUCH_X_1,
	TOUCH_Y_1,
	TOUCH_X_2,
	TOUCH_Y_2,
	MOUSE_X,
	MOUSE_Y,
	MOUSE_VELOCITY_X,
	MOUSE_VELOCITY_Y,
	SCROLL_VELOCITY_X,
	SCROLL_VELOCITY_Y,
};