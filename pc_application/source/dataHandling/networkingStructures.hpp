#pragma once

#include <cstdint>
#include <zpp.hpp>

#include "buttonData.hpp"

// Needed for ZPP
// clang-format off
#define SERIALIZE_DATA(...)  \
	friend zpp::serializer::access; \
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) { \
		archive(__VA_ARGS__); \
	}
// clang-format on

enum DataFlag : uint8_t {
	SET_PROJECT_NAME,
	SET_CURRENT_FRAME,
	MODIFY_FRAME,
	SET_GAME_INFO,
	NUM_OF_FLAGS,
};

namepace Protocol {
	// Structs for the sent and recieved data
	struct SetProjectName : public zpp::serializer::polymorphic {
		// The DataFlag, never included in serialization
		DataFlag flag = DataFlag::SET_PROJECT_NAME;

		std::string name;

		SERIALIZE_DATA(self.name)
	};

	struct SetCurrentFrame : public zpp::serializer::polymorphic {
		// The DataFlag, never included in serialization
		DataFlag flag = DataFlag::SET_CURRENT_FRAME;

		uint32_t frame;

		SERIALIZE_DATA(self.frame)
	};

	struct ModifyFrame : public zpp::serializer::polymorphic {
		// The DataFlag, never included in serialization
		DataFlag flag = DataFlag::MODIFY_FRAME;

		uint32_t frame;
		ControllerData controllerData;

		SERIALIZE_DATA(self.frame, self.controllerData)
	};
};