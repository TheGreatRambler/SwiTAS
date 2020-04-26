#pragma once

#define SET_BIT(number, bit, loc) (number) ^= (-(unsigned long)(bit) ^ (number)) & (1UL << (loc))
#define GET_BIT(number, loc) ((number) >> (loc)) & 1U

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
extern "C" {
#include <jpeglib.h>
}
#include <plog/Log.h>
#include <string>
#include <switch.h>
#include <vector>

// Many thanks to 黯然的饭#8969 on Discord for the framebuffer implementation
class ScreenshotHandler {
private:
	// Different framebuffers are avaliable within VI
	enum FramebufferType : uint8_t {
		FOREGROUND,
		APPLET,
		UNK1,
		HOME1,
		HOME2,
	};

	Result rc;

	const char* tempScreenshotName = "/VI_SCREENSHOT_TEMP.jpg";

	u64 VI_pid;
	// The Title Id of VI (The display manager)
	static constexpr u64 VITitleId = 0x010000000000002D;
	Handle VIdbg;
	// Size of the framebuffer
	// Width * Height * Byte depth (1280 * 720 * 4)
	static constexpr uint16_t framebufferWidth  = 1280;
	static constexpr uint16_t framebufferHeight = 720;
	static constexpr uint8_t bytesPerPixel      = 4;

	static constexpr uint64_t framebufferSize = framebufferWidth * framebufferHeight * bytesPerPixel;
	static constexpr uint16_t rowSize         = framebufferWidth * bytesPerPixel;

	static constexpr uint8_t jpegBytesPerPixel            = 3;
	static constexpr uint16_t jpegFramebufferScanlineSize = framebufferWidth * jpegBytesPerPixel;

	// heightOfdhashInput is also the number of scanlines per read
	// It doesn't really matter if the width and height are similar
	// The number of bytes per hash is 117 with this, 116.25 to be precise
	static constexpr uint8_t heightOfdhashInput = 30;
	static constexpr uint8_t widthOfdhashInput  = 32;
	static constexpr uint8_t sizeOfDhash        = 117;

	// Need to find this out
	static constexpr int jpegQuality = 85;

	// Array of pointers to the start of each scanline
	uint8_t* row_pointer[heightOfdhashInput];

	// Pointer to framebuffer data in VI
	uint64_t framebufferPointer;

	static std::string convertToHexString(uint8_t* data, uint16_t size);

public:
	ScreenshotHandler();

	// Returns a dHash (to determine similarity) and a jpegBuffer to actually view
	void writeFramebuffer(std::string* hash, std::vector<uint8_t>* jpegBuffer);

	~ScreenshotHandler();
};