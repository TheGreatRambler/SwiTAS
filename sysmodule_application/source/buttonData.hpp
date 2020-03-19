#pragma once

#include "../../sharedNetworkCode/buttonData.hpp"
#include <cstdint>
#include <unordered_map>
#include <zpp.hpp>

// Convert my button mappings to the ones recognized by the switch
const std::unordered_map<Btn, HidControllerKeys> btnToHidKeys {
	{ A, KEY_A },
	{ B, KEY_B },
	{ X, KEY_X },
	{ Y, KEY_Y },
	{ L, KEY_L },
	{ R, KEY_R },
	{ ZL, KEY_ZL },
	{ ZR, KEY_ZR },
	{ SL, KEY_SL },
	{ SR, KEY_SR },
	{ DUP, KEY_DUP },
	{ DDOWN, KEY_DDOWN },
	{ DLEFT, KEY_DLEFT },
	{ DRIGHT, KEY_DRIGHT },
	{ PLUS, KEY_PLUS },
	{ MINUS, KEY_MINUS },
	{ HOME, KEY_HOME },
	{ CAPT, KEY_CAPTURE },
	{ LS, KEY_LSTICK },
	{ RS, KEY_RSTICK },
};