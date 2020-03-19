#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <switch.h>
#include <unordered_map>

#include "savestate_data.hpp"
#include "tiny_bmp_parser.hpp"

// Many thanks to 黯然的饭#8969 for the framebuffer implementation
class SavestateHandler {
private:
	// Different framebuffers are avaliable within VI
	enum FramebufferType : uint8_t {
		FOREGROUND,
		APPLET,
		UNK1,
		HOME1,
		HOME2,
	};

	u64 VI_pid;
	// The Title Id of VI (The display manager)
	static constexpr u64 VITitleId = 0x010000000000002D;
	Handle VIdbg;
	// Size of the framebuffer
	// Width * Height * Byte depth (1280 * 720 * 4)
	static constexpr u64 framebufferSize = 3686400;

	// Pointer to framebuffer data in VI
	uint64_t framebufferPointer;

	// The project folder for writing of savestates
	std::string projectFolder;

	// Savestate hooks, not the savestates themselves
	std::unordered_map<uint32_t, SavestateHook> savestateHooks;

	// Current frame
	uint32_t currentFrame;

	// The savestates themselves
	// std::unordered_map<uint32_t, SavestateHook> savestateHooks;

public:
	SavestateHandler();

	void setProjectFolder(std::string folder);

	void createSavestateHookHere();

	void setCurrentFrame(uint32_t frame);

	// Writes as BMP so you can debug if you want
	// Specifically chosen because a bmp is uncompressed
	void writeFramebuffer(BmpParser* bmp);

	~SavestateHandler();
};