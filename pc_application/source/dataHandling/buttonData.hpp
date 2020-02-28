#pragma once

#include <bitset>
#include <cstdio>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <unordered_map>
#include <utility>
#include <wx/wx.h>
#include <zpp.hpp>

#include "../helpers.hpp"

// Array including all button information
// Index is button Id, returns Button script name, Button print name, Button
// listview object, Button printing pixbuf when on and when off This info is
// contained in a struct

// This data is obtained via a JSON file

class ButtonData {
private:
	// Helper function to add transparency masks
	void maskifyBitmap(wxBitmap* bitmap, wxColour maskColor);

public:
	// Buttons enum (coencides with the index of the bit in the input struct)
	// Also used to identify the button everywhere else in the program
	enum Btn : uint8_t {
		A,
		B,
		X,
		Y,
		L,
		R,
		ZL,
		ZR,
		SL,
		SR,
		DUP,
		DDOWN,
		DLEFT,
		DRIGHT,
		PLUS,
		MINUS,
		HOME,
		CAPT,
		LS,
		RS,
		BUTTONS_SIZE,
	};

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

	// Controller data that will be packed into the array and will be recieved from
	// the switch
	struct ControllerData : public zpp::serializer::polymorphic {
		// This controller's index
		uint8_t index;
		// Button data (stored as a bitset because it will be serialized better later)
		// 20 buttons
		// This storage can be sacrificed for the simple reason that the other data
		// already takes up so much space
		uint8_t buttons[20];
		// Joystick values
		int16_t LS_X = 0;
		int16_t LS_Y = 0;
		int16_t RS_X = 0;
		int16_t RS_Y = 0;
		// Gyroscope and Accelerometer data (when it is implemented)
		int16_t ACCEL_X = 0;
		int16_t ACCEL_Y = 0;
		int16_t ACCEL_Z = 0;
		int16_t GYRO_1  = 0;
		int16_t GYRO_2  = 0;
		int16_t GYRO_3  = 0;

		friend zpp::serializer::access;
		template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
			// clang-format off
			archive(self.index, self.buttons,
				self.LS_X, self.LS_Y, self.RS_X, self.RS_Y,
				self.ACCEL_X, self.ACCEL_Y, self.ACCEL_Z,
				self.GYRO_1, self.GYRO_2, self.GYRO_3);
			// clang-format on
		}
	};

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

	const uint8_t KeyWidth  = 11;
	const uint8_t KeyHeight = 4;

	// TODO finish these
	// https://gitlab.gnome.org/GNOME/gtk/blob/master/gdk/gdkkeysyms.h
	std::map<Btn, std::shared_ptr<ButtonInfo>> buttonMapping;

	void setupButtonMapping(rapidjson::Document* mainSettings);
};

// Some good typedef's

typedef ButtonData::Btn Btn;
typedef ButtonData::ControllerData ControllerData;
typedef ButtonData::ButtonInfo ButtonInfo;