#pragma once

#define SET_BIT(number, bit, loc) (number) ^= (-(unsigned long)(bit) ^ (number)) & (1UL << (loc))
#define GET_BIT(number, loc) ((number) >> (loc)) & 1U

#define JPEG_BUF_SIZE 0x80000

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <plog/Log.h>
#include <string>
#include <switch.h>
#include <vector>

#include "../../sharedNetworkCode/networkInterface.hpp"

// Many thanks to 黯然的饭#8969 on Discord for the framebuffer implementation
class ScreenshotHandler {
private:
	Result rc;

public:
	ScreenshotHandler();

	// Returns a dHash (to determine similarity) and a jpegBuffer to actually view
	void writeFramebuffer(std::shared_ptr<CommunicateWithNetwork> networkInstance, uint8_t linkedWithFrameAdvance, uint32_t frame, uint16_t savestateHookNum, uint8_t playerIndex);

	~ScreenshotHandler();
};