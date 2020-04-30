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