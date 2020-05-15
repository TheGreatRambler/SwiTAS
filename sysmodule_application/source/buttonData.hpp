#pragma once

#include "../../sharedNetworkCode/buttonData.hpp"
#include <cstdint>
#include <unordered_map>
#include <zpp.hpp>

// Convert my button mappings to the ones recognized by the switch
const std::unordered_map<Btn, HidControllerKeys> btnToHidKeys{
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

/*
const std::unordered_map<HidControllerKeys, Btn> hidKeysToBtn{
	{ KEY_A, Btn::A },
	{ KEY_B, Btn::B },
	{ KEY_X, Btn::X },
	{ KEY_Y, Btn::Y },
	{ KEY_L, Btn::L },
	{ KEY_R, Btn::R },
	{ KEY_ZL, Btn::ZL },
	{ KEY_ZR, Btn::ZR },
	{ KEY_SL, Btn::SL },
	{ KEY_SR, Btn::SR },
	{ KEY_DUP, Btn::DUP },
	{ KEY_DDOWN, Btn::DDOWN },
	{ KEY_DLEFT, Btn::DLEFT },
	{ KEY_DRIGHT, Btn::DRIGHT },
	{ KEY_PLUS, Btn::PLUS },
	{ KEY_MINUS, Btn::MINUS },
	{ KEY_HOME, Btn::HOME },
	{ KEY_CAPTURE, Btn::CAPT },
	{ KEY_LSTICK, Btn::LS },
	{ KEY_RSTICK, Btn::RS },
};
*/