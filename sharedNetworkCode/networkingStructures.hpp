#pragma once

#include "include/zpp.hpp"
#include <cstdint>
#include <memory>

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
	SendFrameData,
	RecieveGameFramebuffer,
	RecieveGameInfo,
	SendFlag,
	SendAutoRun,
	SendLogging,
	SendTrackMemoryRegion,
	SendSetNumControllers,
	SendAddMemoryRegion,
	SendStartFinalTas,
	RecieveMemoryRegion,
	RecieveLogging,
	RecieveFlag,
	RecieveApplicationConnected,
	RecieveGameMemoryInfo,
	RecieveAutoRunControllerData,
	NUM_OF_FLAGS,
};

enum RecieveInfo : uint8_t {
	RUN_FRAME_DONE,
	FRAMEBUFFER_DONE,
	APPLICATION_DISCONNECTED,
	CONTROLLERS_CONNECTED,
	UNEXPECTED_CONTROLLER_SIZE,
	IS_YUZU,
	IS_HOMEBREW,
};

enum SendInfo : uint8_t {
	GET_FRAMEBUFFER,
	GET_GAME_INFO,
	START_TAS_MODE,
	END_TAS_MODE,
	PAUSE_DEBUG,
	UNPAUSE_DEBUG,
	// These methods aren't debug because they are called by savestateHandler
	PAUSE,
	UNPAUSE,
	RUN_BLANK_FRAME,
	STOP_FULL_SPEED,
	PAUSE_FULL_SPEED,
	STOP_FINAL_TAS,
	IS_YUZU,
};

// This is used by the switch to determine size, a vector is always send back enyway
enum MemoryRegionTypes : uint8_t {
	Bit8 = 0,
	Bit16,
	Bit32,
	Bit64,
	Float,
	Double,
	Bool,
	CharPointer,
	ByteArray,
	NUM_OF_TYPES,
};

// clang-format off
namespace Protocol {
	// Run a single frame and return when done
	DEFINE_STRUCT(SendFrameData,
		ControllerData controllerData;
		// This is the frame of this frame, the framebuffer
		// sent back will be one more than this frame
		uint32_t frame;
		uint16_t savestateHookNum;
		uint16_t branchIndex;
		uint8_t playerIndex;
		uint8_t incrementFrame;
		uint8_t includeFramebuffer;
		uint8_t isAutoRun;
	, self.controllerData, self.frame, self.playerIndex, self.incrementFrame, self.branchIndex, self.savestateHookNum, self.includeFramebuffer, self.isAutoRun)

	// Recieve all of the game's framebuffer
	DEFINE_STRUCT(RecieveGameFramebuffer,
		std::vector<uint8_t> buf;
		uint8_t fromFrameAdvance;
		uint32_t frame;
		uint16_t savestateHookNum;
		uint16_t branchIndex;
		uint8_t playerIndex;
		// Set by auto advance
		uint8_t controllerDataIncluded;
		ControllerData controllerData;
	, self.buf, self.fromFrameAdvance, self.frame, self.savestateHookNum, self.branchIndex, self.playerIndex, self.controllerDataIncluded, self.controllerData)

	// Recieve a ton of game and user info
	DEFINE_STRUCT(RecieveGameInfo,
	/*
		std::string applicationName;
		uint64_t applicationProgramId;
		uint64_t applicationProcessId;
		//std::string userNickname;
		std::vector<GameMemoryInfo> memoryInfo;
		*/
		// Info in the form of a string
		std::string infoJson;
	, self.infoJson)

	// Send start, with mostly everything as an enum value
	DEFINE_STRUCT(SendFlag,
		SendInfo actFlag;
	, self.actFlag)

	// Needs to have number of controllers set right, TODO
	DEFINE_STRUCT(SendStartFinalTas,
		std::vector<std::string> scriptPaths;
	, self.scriptPaths)

	DEFINE_STRUCT(SendLogging,
		std::string log;
	, self.log)

	DEFINE_STRUCT(SendTrackMemoryRegion,
		uint64_t startByte;
		uint64_t size;
	, self.startByte, self.size)

	DEFINE_STRUCT(SendAddMemoryRegion,
		std::string pointerDefinition;
		MemoryRegionTypes type;
		uint8_t clearAllRegions;
		uint8_t u;
		uint64_t dataSize;
	, self.pointerDefinition, self.type, self.clearAllRegions, self.u, self.dataSize)

	DEFINE_STRUCT(SendSetNumControllers,
		uint8_t size;
	, self.size)

	DEFINE_STRUCT(RecieveMemoryRegion,
		std::vector<uint8_t> memory;
		std::string stringRepresentation;
		uint16_t index;
	, self.memory, self.stringRepresentation, self.index)

	DEFINE_STRUCT(RecieveLogging,
		std::string log;
	, self.log)

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