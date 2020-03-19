#pragma once

#include "include/zpp.hpp"
#include <cstdint>

#include "buttonData.hpp"

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
	SendRunFrame,
	RecieveGameFramebuffer,
	RecieveGameInfo,
	SendFlag,
	RecieveFlag,
	RecieveApplicationConnected,
	NUM_OF_FLAGS,
};

enum RecieveInfo : uint8_t {
	RUN_FRAME_DONE,
	FRAMEBUFFER_DONE,
	APPLICATION_DISCONNECTED,
};

enum SendInfo : uint8_t {
	GET_FRAMEBUFFER,
	GET_GAME_INFO,
};

// clang-format off
namespace Protocol {
	// Run a single frame and return when done
	DEFINE_STRUCT(SendRunFrame,
		ControllerData controllerData;
	, self.controllerData)

	// Recieve part of the game's framebuffer
	DEFINE_STRUCT(RecieveGameFramebuffer,
		std::vector<uint8_t> buf;
		std::string dHash;
	, self.buf, self.dHash)

	// Recieve a ton of game and user info
	DEFINE_STRUCT(RecieveGameInfo,
		std::string userNickname;
	, self.userNickname)

	// Send start, with mostly everything as an enum value
	DEFINE_STRUCT(SendFlag,
		SendInfo actFlag;
	, self.actFlag)

	// Recieve done, with mostly everything as an enum value
	DEFINE_STRUCT(RecieveFlag,
		RecieveInfo actFlag;
	, self.actFlag)

	DEFINE_STRUCT(RecieveApplicationConnected,
		std::string applicationName;
		uint64_t applicationProgramId;
		uint64_t applicationProcessId;
	, self.applicationName, self.applicationProgramId, self.applicationProcessId)
};
// clang-format on