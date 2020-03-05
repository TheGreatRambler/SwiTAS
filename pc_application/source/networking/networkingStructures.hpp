#pragma once

#include <cstdint>
#include <zpp.hpp>

#include "../dataHandling/buttonData.hpp"

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
	RecieveGameFramebuffer,
	RecieveGameInfo,
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

	DEFINE_STRUCT(RecieveGameFramebuffer,
		std::vector<uint8_t> frameBuffer;
	, self.frameBuffer)

	DEFINE_STRUCT(RecieveGameInfo,
		std::string userNickname;
	, self.userNickname)
};
// clang-format on