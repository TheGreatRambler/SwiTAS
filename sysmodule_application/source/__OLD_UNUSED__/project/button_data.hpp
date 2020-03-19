#pragma once

#include <bitset>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <zpp.hpp>

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

// Controller data that will be packed into the array and will be recieved from the PC
struct InputData {
	// This controller's index
	uint8_t controllerIndex = 0;
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

	// The serializer method
	friend zpp::serializer::access;
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
		archive(self.controllerIndex, self.buttons, self.LS_X, self.LS_Y, self.RS_X, self.RS_Y, self.ACCEL_X, self.ACCEL_Y, self.ACCEL_Z, self.GYRO_1, self.GYRO_2, self.GYRO_3);
	}
};

// The structure representing a page of 60 inputs that is loaded when needed
struct InputsPage {
	// The number of accesses since this page has been read,
	//  once it reaches a constant, the page is closed to save memory
	// When the file is closed, all inputs associated with it are written to disk
	uint8_t accessesSinceLastRead;
	// The file descriptor is not included
	// The inputs that represent it
	// 60 is a good number; 60 frames is one second
	// The index is the offset from the start of the page
	std::unordered_map<uint8_t, std::shared_ptr<InputData>> inputs;
};