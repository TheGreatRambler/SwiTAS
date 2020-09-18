#pragma once

#include <bitset>
#include <cstdio>
#include <cstring>
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
#include "../shared_network_code/buttonData.hpp"
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

	std::map<std::string, nn::hid::KeyboardKey> stringToKeyboardKey {
		{ "A", nn::hid::KeyboardKey::A },
		{ "B", nn::hid::KeyboardKey::B },
		{ "C", nn::hid::KeyboardKey::C },
		{ "D", nn::hid::KeyboardKey::D },
		{ "E", nn::hid::KeyboardKey::E },
		{ "F", nn::hid::KeyboardKey::F },
		{ "G", nn::hid::KeyboardKey::G },
		{ "H", nn::hid::KeyboardKey::H },
		{ "I", nn::hid::KeyboardKey::I },
		{ "J", nn::hid::KeyboardKey::J },
		{ "K", nn::hid::KeyboardKey::K },
		{ "L", nn::hid::KeyboardKey::L },
		{ "M", nn::hid::KeyboardKey::M },
		{ "N", nn::hid::KeyboardKey::N },
		{ "O", nn::hid::KeyboardKey::O },
		{ "P", nn::hid::KeyboardKey::P },
		{ "Q", nn::hid::KeyboardKey::Q },
		{ "R", nn::hid::KeyboardKey::R },
		{ "S", nn::hid::KeyboardKey::S },
		{ "T", nn::hid::KeyboardKey::T },
		{ "U", nn::hid::KeyboardKey::U },
		{ "V", nn::hid::KeyboardKey::V },
		{ "W", nn::hid::KeyboardKey::W },
		{ "X", nn::hid::KeyboardKey::X },
		{ "Y", nn::hid::KeyboardKey::Y },
		{ "Z", nn::hid::KeyboardKey::Z },
		{ "N1", nn::hid::KeyboardKey::N1 },
		{ "N2", nn::hid::KeyboardKey::N2 },
		{ "N3", nn::hid::KeyboardKey::N3 },
		{ "N4", nn::hid::KeyboardKey::N4 },
		{ "N5", nn::hid::KeyboardKey::N5 },
		{ "N6", nn::hid::KeyboardKey::N6 },
		{ "N7", nn::hid::KeyboardKey::N7 },
		{ "N8", nn::hid::KeyboardKey::N8 },
		{ "N9", nn::hid::KeyboardKey::N9 },
		{ "N0", nn::hid::KeyboardKey::N0 },
		{ "Enter", nn::hid::KeyboardKey::Enter },
		{ "Escape", nn::hid::KeyboardKey::Escape },
		{ "Backspace", nn::hid::KeyboardKey::Backspace },
		{ "Tab", nn::hid::KeyboardKey::Tab },
		{ "Space", nn::hid::KeyboardKey::Space },
		{ "Minus", nn::hid::KeyboardKey::Minus },
		{ "Equal", nn::hid::KeyboardKey::Equal },
		{ "LeftBrace", nn::hid::KeyboardKey::LeftBrace },
		{ "RightBrace", nn::hid::KeyboardKey::RightBrace },
		{ "Backslash", nn::hid::KeyboardKey::Backslash },
		{ "Tilde", nn::hid::KeyboardKey::Tilde },
		{ "Semicolon", nn::hid::KeyboardKey::Semicolon },
		{ "Apostrophe", nn::hid::KeyboardKey::Apostrophe },
		{ "Grave", nn::hid::KeyboardKey::Grave },
		{ "Comma", nn::hid::KeyboardKey::Comma },
		{ "Dot", nn::hid::KeyboardKey::Dot },
		{ "Slash", nn::hid::KeyboardKey::Slash },
		{ "CapsLockKey", nn::hid::KeyboardKey::CapsLockKey },
		{ "F1", nn::hid::KeyboardKey::F1 },
		{ "F2", nn::hid::KeyboardKey::F2 },
		{ "F3", nn::hid::KeyboardKey::F3 },
		{ "F4", nn::hid::KeyboardKey::F4 },
		{ "F5", nn::hid::KeyboardKey::F5 },
		{ "F6", nn::hid::KeyboardKey::F6 },
		{ "F7", nn::hid::KeyboardKey::F7 },
		{ "F8", nn::hid::KeyboardKey::F8 },
		{ "F9", nn::hid::KeyboardKey::F9 },
		{ "F10", nn::hid::KeyboardKey::F10 },
		{ "F11", nn::hid::KeyboardKey::F11 },
		{ "F12", nn::hid::KeyboardKey::F12 },
		{ "SystemRequest", nn::hid::KeyboardKey::SystemRequest },
		{ "ScrollLockKey", nn::hid::KeyboardKey::ScrollLockKey },
		{ "Pause", nn::hid::KeyboardKey::Pause },
		{ "Insert", nn::hid::KeyboardKey::Insert },
		{ "Home", nn::hid::KeyboardKey::Home },
		{ "PageUp", nn::hid::KeyboardKey::PageUp },
		{ "Delete", nn::hid::KeyboardKey::Delete },
		{ "End", nn::hid::KeyboardKey::End },
		{ "PageDown", nn::hid::KeyboardKey::PageDown },
		{ "Right", nn::hid::KeyboardKey::Right },
		{ "Left", nn::hid::KeyboardKey::Left },
		{ "Down", nn::hid::KeyboardKey::Down },
		{ "Up", nn::hid::KeyboardKey::Up },
		{ "NumLockKey", nn::hid::KeyboardKey::NumLockKey },
		{ "KPSlash", nn::hid::KeyboardKey::KPSlash },
		{ "KPAsterisk", nn::hid::KeyboardKey::KPAsterisk },
		{ "KPMinus", nn::hid::KeyboardKey::KPMinus },
		{ "KPPlus", nn::hid::KeyboardKey::KPPlus },
		{ "KPEnter", nn::hid::KeyboardKey::KPEnter },
		{ "KP1", nn::hid::KeyboardKey::KP1 },
		{ "KP2", nn::hid::KeyboardKey::KP2 },
		{ "KP3", nn::hid::KeyboardKey::KP3 },
		{ "KP4", nn::hid::KeyboardKey::KP4 },
		{ "KP5", nn::hid::KeyboardKey::KP5 },
		{ "KP6", nn::hid::KeyboardKey::KP6 },
		{ "KP7", nn::hid::KeyboardKey::KP7 },
		{ "KP8", nn::hid::KeyboardKey::KP8 },
		{ "KP9", nn::hid::KeyboardKey::KP9 },
		{ "KP0", nn::hid::KeyboardKey::KP0 },
		{ "KPDot", nn::hid::KeyboardKey::KPDot },
		{ "Key102", nn::hid::KeyboardKey::Key102 },
		{ "Compose", nn::hid::KeyboardKey::Compose },
		{ "Power", nn::hid::KeyboardKey::Power },
		{ "KPEqual", nn::hid::KeyboardKey::KPEqual },
		{ "F13", nn::hid::KeyboardKey::F13 },
		{ "F14", nn::hid::KeyboardKey::F14 },
		{ "F15", nn::hid::KeyboardKey::F15 },
		{ "F16", nn::hid::KeyboardKey::F16 },
		{ "F17", nn::hid::KeyboardKey::F17 },
		{ "F18", nn::hid::KeyboardKey::F18 },
		{ "F19", nn::hid::KeyboardKey::F19 },
		{ "F20", nn::hid::KeyboardKey::F20 },
		{ "F21", nn::hid::KeyboardKey::F21 },
		{ "F22", nn::hid::KeyboardKey::F22 },
		{ "F23", nn::hid::KeyboardKey::F23 },
		{ "F24", nn::hid::KeyboardKey::F24 },
		{ "Open", nn::hid::KeyboardKey::Open },
		{ "Help", nn::hid::KeyboardKey::Help },
		{ "Properties", nn::hid::KeyboardKey::Properties },
		{ "Front", nn::hid::KeyboardKey::Front },
		{ "Stop", nn::hid::KeyboardKey::Stop },
		{ "Repeat", nn::hid::KeyboardKey::Repeat },
		{ "Undo", nn::hid::KeyboardKey::Undo },
		{ "Cut", nn::hid::KeyboardKey::Cut },
		{ "Copy", nn::hid::KeyboardKey::Copy },
		{ "Paste", nn::hid::KeyboardKey::Paste },
		{ "Find", nn::hid::KeyboardKey::Find },
		{ "Mute", nn::hid::KeyboardKey::Mute },
		{ "VolumeUp", nn::hid::KeyboardKey::VolumeUp },
		{ "VolumeDown", nn::hid::KeyboardKey::VolumeDown },
		{ "CapsLockActive", nn::hid::KeyboardKey::CapsLockActive },
		{ "NumLockActive", nn::hid::KeyboardKey::NumLockActive },
		{ "ScrollLockActive", nn::hid::KeyboardKey::ScrollLockActive },
		{ "KPComma", nn::hid::KeyboardKey::KPComma },
		{ "KPLeftParenthesis", nn::hid::KeyboardKey::KPLeftParenthesis },
		{ "KPRightParenthesis", nn::hid::KeyboardKey::KPRightParenthesis },
		{ "LeftControlKey", nn::hid::KeyboardKey::LeftControlKey },
		{ "LeftShiftKey", nn::hid::KeyboardKey::LeftShiftKey },
		{ "LeftAltKey", nn::hid::KeyboardKey::LeftAltKey },
		{ "LeftMetaKey", nn::hid::KeyboardKey::LeftMetaKey },
		{ "RightControlKey", nn::hid::KeyboardKey::RightControlKey },
		{ "RightShiftKey", nn::hid::KeyboardKey::RightShiftKey },
		{ "RightAltKey", nn::hid::KeyboardKey::RightAltKey },
		{ "RightMetaKey", nn::hid::KeyboardKey::RightMetaKey },
		{ "MediaPlayPause", nn::hid::KeyboardKey::MediaPlayPause },
		{ "MediaStopCD", nn::hid::KeyboardKey::MediaStopCD },
		{ "MediaPrevious", nn::hid::KeyboardKey::MediaPrevious },
		{ "MediaNext", nn::hid::KeyboardKey::MediaNext },
		{ "MediaEject", nn::hid::KeyboardKey::MediaEject },
		{ "MediaVolumeUp", nn::hid::KeyboardKey::MediaVolumeUp },
		{ "MediaVolumeDown", nn::hid::KeyboardKey::MediaVolumeDown },
		{ "MediaMute", nn::hid::KeyboardKey::MediaMute },
		{ "MediaWebsite", nn::hid::KeyboardKey::MediaWebsite },
		{ "MediaBack", nn::hid::KeyboardKey::MediaBack },
		{ "MediaForward", nn::hid::KeyboardKey::MediaForward },
		{ "MediaStop", nn::hid::KeyboardKey::MediaStop },
		{ "MediaFind", nn::hid::KeyboardKey::MediaFind },
		{ "MediaScrollUp", nn::hid::KeyboardKey::MediaScrollUp },
		{ "MediaScrollDown", nn::hid::KeyboardKey::MediaScrollDown },
		{ "MediaEdit", nn::hid::KeyboardKey::MediaEdit },
		{ "MediaSleep", nn::hid::KeyboardKey::MediaSleep },
		{ "MediaCoffee", nn::hid::KeyboardKey::MediaCoffee },
		{ "MediaRefresh", nn::hid::KeyboardKey::MediaRefresh },
		{ "MediaCalculator", nn::hid::KeyboardKey::MediaCalculator },
		{ "NumKeyboardKeys", nn::hid::KeyboardKey::NumKeyboardKeys },
	};

	std::map<std::string, nn::hid::KeyboardModifier> stringToKeyboardModifier {
		{ "CapsLock", nn::hid::KeyboardModifier::CapsLock },
		{ "Control", nn::hid::KeyboardModifier::Control },
		{ "Gui", nn::hid::KeyboardModifier::Gui },
		{ "LeftAlt", nn::hid::KeyboardModifier::LeftAlt },
		{ "NumLock", nn::hid::KeyboardModifier::NumLock },
		{ "RightAlt", nn::hid::KeyboardModifier::RightAlt },
		{ "ScrollLock", nn::hid::KeyboardModifier::ScrollLock },
		{ "Shift", nn::hid::KeyboardModifier::Shift },
	};

	std::map<std::string, nn::hid::MouseButton> stringToMouseButton {
		{ "Back", nn::hid::MouseButton::Back },
		{ "Forward", nn::hid::MouseButton::Forward },
		{ "Left", nn::hid::MouseButton::Left },
		{ "Middle", nn::hid::MouseButton::Middle },
		{ "Right", nn::hid::MouseButton::Right },
	};

	std::unordered_map<std::string, Btn> scriptNameToButton;
	std::unordered_map<nn::hid::KeyboardKey, std::string> keyboardKeyToString;
	std::unordered_map<nn::hid::KeyboardModifier, std::string> keyboardModifierToString;
	std::unordered_map<nn::hid::MouseButton, std::string> mouseButtonToString;

	static constexpr int16_t axisMin = -32767;
	static constexpr int16_t axisMax = 32767;

	static constexpr uint8_t KeyWidth  = 11;
	static constexpr uint8_t KeyHeight = 4;

	std::map<Btn, std::shared_ptr<ButtonInfo>> buttonMapping;
	// TODO create corresponding mapping for keyboard, keyboard modifier and mouse buttons
	std::map<nn::hid::KeyboardKey, std::shared_ptr<KeyboardKeyInfo>> keyboardKeyMapping;
	std::map<nn::hid::KeyboardModifier, std::shared_ptr<KeyboardModifierKeyInfo>> keyboardModifierKeyMapping;
	std::map<nn::hid::MouseButton, std::shared_ptr<MouseButtonInfo>> mouseButtonMapping;

	void setupButtonMapping(rapidjson::Document* mainSettings);

	FrameNum textToFrames(DataProcessing* dataProcessing, std::string text, FrameNum startLoc, bool insertPaste, bool placePaste);
	std::string framesToText(DataProcessing* dataProcessing, FrameNum startLoc, FrameNum endLoc, int playerIndex, BranchNum branch);

	void transferControllerData(ControllerData src, std::shared_ptr<ControllerData> dest, bool placePaste);
	void transferExtraData(TouchAndKeyboardData src, std::shared_ptr<TouchAndKeyboardData> dest, bool placePaste);
	void transferOnlyKeyboard(TouchAndKeyboardData src, std::shared_ptr<TouchAndKeyboardData> dest);
	void transferOnlyTouch(TouchAndKeyboardData src, std::shared_ptr<TouchAndKeyboardData> dest);

	bool isEmptyControllerData(std::shared_ptr<ControllerData> data);
	bool isEmptyExtraData(std::shared_ptr<TouchAndKeyboardData> data);

	std::string getKeyboardKeysString(std::shared_ptr<TouchAndKeyboardData> data);
	std::string getKeyboardModifiersString(std::shared_ptr<TouchAndKeyboardData> data);
	std::string getMouseButtonsString(std::shared_ptr<TouchAndKeyboardData> data);
};