#pragma once

#include "shared_network_code/buttonData.hpp"
#include <cstdint>
#include <unordered_map>
#include <zpp.hpp>

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef YUZU
#include "yuzuSyscalls.hpp"
#endif

// Convert my button mappings to the ones recognized by the switch
#ifdef __SWITCH__
const std::unordered_map<Btn, HidControllerKeys> btnToHidKeys {
	{ Btn::A, KEY_A },
	{ Btn::B, KEY_B },
	{ Btn::X, KEY_X },
	{ Btn::Y, KEY_Y },
	{ Btn::L, KEY_L },
	{ Btn::R, KEY_R },
	{ Btn::ZL, KEY_ZL },
	{ Btn::ZR, KEY_ZR },
	{ Btn::SL, KEY_SL },
	{ Btn::SR, KEY_SR },
	{ Btn::DUP, KEY_DUP },
	{ Btn::DDOWN, KEY_DDOWN },
	{ Btn::DLEFT, KEY_DLEFT },
	{ Btn::DRIGHT, KEY_DRIGHT },
	{ Btn::PLUS, KEY_PLUS },
	{ Btn::MINUS, KEY_MINUS },
	{ Btn::HOME, KEY_HOME },
	{ Btn::CAPT, KEY_CAPTURE },
	{ Btn::LS, KEY_LSTICK },
	{ Btn::RS, KEY_RSTICK },
};
#else
using BV = PluginDefinitions::ButtonValues;
const std::unordered_map<Btn, BV> btnToHidKeys {
	{ Btn::A, BV::A },
	{ Btn::B, BV::B },
	{ Btn::X, BV::X },
	{ Btn::Y, BV::Y },
	{ Btn::L, BV::L },
	{ Btn::R, BV::R },
	{ Btn::ZL, BV::ZL },
	{ Btn::ZR, BV::ZR },
	{ Btn::SL, BV::SL },
	{ Btn::SR, BV::SR },
	{ Btn::DUP, BV::DUp },
	{ Btn::DDOWN, BV::DDown },
	{ Btn::DLEFT, BV::DLeft },
	{ Btn::DRIGHT, BV::DRight },
	{ Btn::PLUS, BV::Plus },
	{ Btn::MINUS, BV::Minus },
	{ Btn::HOME, BV::Home },
	{ Btn::CAPT, BV::Screenshot },
	{ Btn::LS, BV::LStick },
	{ Btn::RS, BV::RStick },
};
#endif

const std::unordered_map<Btn, std::string> btnOverlayImageNames {
	{ Btn::A, "A.png" },
	{ Btn::B, "B.png" },
	{ Btn::X, "X.png" },
	{ Btn::Y, "Y.png" },
	{ Btn::L, "L.png" },
	{ Btn::R, "R.png" },
	{ Btn::ZL, "ZL.png" },
	{ Btn::ZR, "ZR.png" },
	{ Btn::SL, "SL.png" },
	{ Btn::SR, "SR.png" },
	{ Btn::DUP, "UP.png" },
	{ Btn::DDOWN, "DOWN.png" },
	{ Btn::DLEFT, "LEFT.png" },
	{ Btn::DRIGHT, "RIGHT.png" },
	{ Btn::PLUS, "PLUS.png" },
	{ Btn::MINUS, "MINUS.png" },
	{ Btn::HOME, "HOME.png" },
	{ Btn::CAPT, "CAPT.png" },
	{ Btn::LS, "LS.png" },
	{ Btn::RS, "RS.png" },
};