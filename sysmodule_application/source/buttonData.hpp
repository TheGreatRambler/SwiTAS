#pragma once

#define SET_BIT(number, bit, loc) (number) ^= (-(unsigned long)(bit) ^ (number)) & (1UL << (loc))
#define GET_BIT(number, loc) ((number) >> (loc)) & 1U

#include <cstdint>
#include <unordered_map>
#include <zpp.hpp>

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

// Controller data that will be packed into the array and will be recieved from
// the switch
struct ControllerData : public zpp::serializer::polymorphic {
	// This controller's index
	uint8_t index;
	// Button data stored with bitflags in 3 bytes
	// 20 flags need to be stored
	uint32_t buttons;
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
	// State of the frame, mostly for the editor
	uint8_t frameState;

	friend zpp::serializer::access;
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
		// clang-format off
			archive(self.index, self.buttons,
				self.LS_X, self.LS_Y, self.RS_X, self.RS_Y,
				self.ACCEL_X, self.ACCEL_Y, self.ACCEL_Z,
				self.GYRO_1, self.GYRO_2, self.GYRO_3,
				self.frameState);
		// clang-format on
	}
};