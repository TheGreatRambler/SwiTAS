#pragma once

#include <cstdint>
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

// clang-format off
#define DEFINE_STRUCT(Flag, body, ...) \
	struct Struct_##Flag : public zpp::serializer::polymorphic { \
		DataFlag flag = DataFlag::Flag; \
		body \
		friend zpp::serializer::access; \
		template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) { \
			archive(__VA_ARGS__); \
		} \
	};
// clang-format on

enum DataFlag : uint8_t {
	SetProjectName,
	SetCurrentFrame,
	ModifyFrame,
	IsPaused,
	NUM_OF_FLAGS,
};

// clang-format off
namespace Protocol {
	DEFINE_STRUCT(SetProjectName,
		std::string name;
	, self.name)

	DEFINE_STRUCT(SetCurrentFrame,
		uint32_t frame;
	, self.frame)

	DEFINE_STRUCT(ModifyFrame,
		uint32_t frame;
		ControllerData controllerData;
	, self.frame, self.controllerData)

	DEFINE_STRUCT(IsPaused,
		uint8_t isPaused;
	, self.isPaused)
};
// clang-format on