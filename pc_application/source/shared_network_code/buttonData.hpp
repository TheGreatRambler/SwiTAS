#pragma once

#define SET_BIT(number, bit, loc) (number) ^= (-(unsigned long)(bit) ^ (number)) & (1UL << (loc))
#define GET_BIT(number, loc) ((number) >> (loc)) & 1U
#define IS_KEYBOARD_HELD(data, key) data[key / 32] & (1 << (key % 32));
// clang-format off
#define SET_KEYBOARD_HELD(data, key, state) \
	if(state) { \
		data[key / 32] |= (1 << (key % 32)); \
	} else { \
		data[key / 32] &= ~(1 << (key % 32)); \
	}
// clang-format on

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
	SAVESTATE,
};

enum TasValueToRecord : uint8_t {
	NONE,
	CONTROLLER,
	KEYBOARD_MOUSE,
	TOUCHSCREEN,
	NUM_OF_VALUES,
};

// Controller data that will be packed into the array and will be recieved from
// the switch
struct ControllerData : public zpp::serializer::polymorphic {
	uint32_t buttons         = 0;
	int16_t LS_X             = 0;
	int16_t LS_Y             = 0;
	int16_t RS_X             = 0;
	int16_t RS_Y             = 0;
	float ACCEL_X_LEFT       = 0.0;
	float ACCEL_Y_LEFT       = 0.0;
	float ACCEL_Z_LEFT       = -1.0;
	float GYRO_X_LEFT        = 0.0;
	float GYRO_Y_LEFT        = 0.0;
	float GYRO_Z_LEFT        = 0.0;
	float ANGLE_X_LEFT       = 0.0;
	float ANGLE_Y_LEFT       = 0.0;
	float ANGLE_Z_LEFT       = 0.0;
	float DIRECTION_XX_LEFT  = 1.0;
	float DIRECTION_XY_LEFT  = 0.0;
	float DIRECTION_XZ_LEFT  = 0.0;
	float DIRECTION_YX_LEFT  = 0.0;
	float DIRECTION_YY_LEFT  = 1.0;
	float DIRECTION_YZ_LEFT  = 0.0;
	float DIRECTION_ZX_LEFT  = 0.0;
	float DIRECTION_ZY_LEFT  = 0.0;
	float DIRECTION_ZZ_LEFT  = 1.0;
	float ACCEL_X_RIGHT      = 0.0;
	float ACCEL_Y_RIGHT      = 0.0;
	float ACCEL_Z_RIGHT      = -1.0;
	float GYRO_X_RIGHT       = 0.0;
	float GYRO_Y_RIGHT       = 0.0;
	float GYRO_Z_RIGHT       = 0.0;
	float ANGLE_X_RIGHT      = 0.0;
	float ANGLE_Y_RIGHT      = 0.0;
	float ANGLE_Z_RIGHT      = 0.0;
	float DIRECTION_XX_RIGHT = 1.0;
	float DIRECTION_XY_RIGHT = 0.0;
	float DIRECTION_XZ_RIGHT = 0.0;
	float DIRECTION_YX_RIGHT = 0.0;
	float DIRECTION_YY_RIGHT = 1.0;
	float DIRECTION_YZ_RIGHT = 0.0;
	float DIRECTION_ZX_RIGHT = 0.0;
	float DIRECTION_ZY_RIGHT = 0.0;
	float DIRECTION_ZZ_RIGHT = 1.0;
	uint8_t frameState       = 0;

	friend zpp::serializer::access;
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
		// clang-format off
			archive(self.buttons,
				self.LS_X, self.LS_Y, self.RS_X, self.RS_Y,
				self.ACCEL_X_LEFT, self.ACCEL_Y_LEFT, self.ACCEL_Z_LEFT,
				self.GYRO_X_LEFT, self.GYRO_Y_LEFT, self.GYRO_Z_LEFT,
				self.ANGLE_X_LEFT, self.ANGLE_Y_LEFT, self.ANGLE_Z_LEFT,
				self.ACCEL_X_RIGHT, self.ACCEL_Y_RIGHT, self.ACCEL_Z_RIGHT,
				self.GYRO_X_RIGHT, self.GYRO_Y_RIGHT, self.GYRO_Z_RIGHT,
				self.ANGLE_X_RIGHT, self.ANGLE_Y_RIGHT, self.ANGLE_Z_RIGHT,
				self.DIRECTION_XX_LEFT, self.DIRECTION_XY_LEFT,
				self.DIRECTION_XZ_LEFT, self.DIRECTION_YX_LEFT,
				self.DIRECTION_YY_LEFT, self.DIRECTION_YZ_LEFT,
				self.DIRECTION_ZX_LEFT, self.DIRECTION_ZY_LEFT,
				self.DIRECTION_ZZ_LEFT, self.DIRECTION_XX_RIGHT,
				self.DIRECTION_XY_RIGHT, self.DIRECTION_XZ_RIGHT,
				self.DIRECTION_YX_RIGHT, self.DIRECTION_YY_RIGHT,
				self.DIRECTION_YZ_RIGHT, self.DIRECTION_ZX_RIGHT,
				self.DIRECTION_ZY_RIGHT, self.DIRECTION_ZZ_RIGHT,
				self.frameState);
		// clang-format on
	}
};

// Stored in parrallel to the rest of the data, just not sharing the player
struct TouchAndKeyboardData : public zpp::serializer::polymorphic {
	int32_t touchX1           = 0;
	int32_t touchY1           = 0;
	int32_t touchX2           = 0;
	int32_t touchY2           = 0;
	uint8_t numberOfTouches   = 0;
	int32_t keyboardModifiers = 0;
	uint32_t keyboardKeys[8];
	int32_t mouseX          = 0;
	int32_t mouseY          = 0;
	int32_t mouseVelocityX  = 0;
	int32_t mouseVelocityY  = 0;
	int32_t scrollVelocityX = 0;
	int32_t scrollVelocityY = 0;
	int32_t mouseButtons    = 0;

	friend zpp::serializer::access;
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
		// clang-format off
			archive(self.touchX1, self.touchY1, self.touchX2, self.touchY2,
				self.keyboardModifiers, self.numberOfTouches,
				self.keyboardKeys, self.mouseX, self.mouseY, self.mouseVelocityX,
				self.mouseVelocityY, self.scrollVelocityX, self.scrollVelocityY,
				self.mouseButtons);
		// clang-format on
	}
};

// Not button data, but the memory info
// Mirrors libnx MemoryInfo
/*
struct GameMemoryInfo {
	uint64_t addr;            ///< Base address.
	uint64_t size;            ///< Size.
	uint32_t type;            ///< Memory type (see lower 8 bits of \ref MemoryState).
	uint32_t attr;            ///< Memory attributes (see \ref MemoryAttribute).
	uint32_t perm;            ///< Memory permissions (see \ref Permission).
	uint32_t device_refcount; ///< Device reference count.
	uint32_t ipc_refcount;    ///< IPC reference count.
	uint32_t padding;         ///< Padding.

	friend zpp::serializer::access;
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
		// clang-format off
			archive(self.addr, self.size, self.type,
				self.attr, self.perm, self.device_refcount,
				self.ipc_refcount, self.padding);
		// clang-format on
	}
};
*/