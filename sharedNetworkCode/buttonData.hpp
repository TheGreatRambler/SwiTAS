#pragma once

#include "include/zpp.hpp"
#include <cstdint>

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

enum FrameState : uint8_t {
	RAN,
	SAVESTATE_HOOK,
};

// Controller data that will be packed into the array and will be recieved from
// the switch
struct ControllerData : public zpp::serializer::polymorphic {
	// This controller's index
	uint8_t index = 0;
	// Button data stored with bitflags in 3 bytes
	// 20 flags need to be stored
	uint32_t buttons = 0;
	// Joystick values
	// https://switchbrew.github.io/libnx/structHiddbgHdlsState.html
	// Range -30000 to 30000
	int32_t LS_X = 0;
	int32_t LS_Y = 0;
	int32_t RS_X = 0;
	int32_t RS_Y = 0;
	// Gyroscope and Accelerometer data (when it is implemented)
	int16_t ACCEL_X = 0;
	int16_t ACCEL_Y = 0;
	int16_t ACCEL_Z = 0;
	int16_t GYRO_1  = 0;
	int16_t GYRO_2  = 0;
	int16_t GYRO_3  = 0;
	// State of the frame, mostly for the editor
	uint8_t frameState = 0;

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