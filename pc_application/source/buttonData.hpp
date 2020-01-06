#pragma once

#include <bitset>
#include <cstdio>
#include <gdkmm/pixbuf.h>
#include <glibmm/fileutils.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/treemodelcolumn.h>
#include <map>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <unordered_map>
#include <utility>

// Array including all button information
// Index is button Id, returns Button script name, Button print name, Button
// listview object, Button printing pixbuf when on and when off This info is
// contained in a struct

// This data is obtained via a JSON file

class ButtonData {
private:
	Glib::RefPtr<Gdk::Pixbuf> getNewIcon(std::string path);

public:
	// Buttons enum (coencides with the index of the bit in the input struct)
	// Also used to identify the button everywhere else in the program
	enum Btn {
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
	struct ControllerData {
		// This controller's index
		uint8_t index;
		// Button data (stored as a bitset because it will be serialized better later)
		// 20 buttons
		// This storage can be sacrificed for the simple reason that the other data
		// already takes up so much space
		std::bitset<20> buttons;
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
	};

	// Struct containing button info
	struct ButtonInfo {
		Glib::ustring scriptName;
		Glib::ustring viewName;
		Glib::RefPtr<Gdk::Pixbuf> onIcon;
		Glib::RefPtr<Gdk::Pixbuf> offIcon;
		guint toggleKeybind;
	};

	// TODO finish these
	// https://gitlab.gnome.org/GNOME/gtk/blob/master/gdk/gdkkeysyms.h
	std::map<Btn, ButtonInfo> buttonMapping;

	void setupButtonMapping(rapidjson::Document* mainSettings);
};

// Some good typedef's

typedef ButtonData::Btn Btn;
typedef ButtonData::ControllerData ControllerData;