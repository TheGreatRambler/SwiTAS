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
#ifdef __SWITCH__
#include <switch.h>
#endif
#include <vector>

#include "../../sharedNetworkCode/networkInterface.hpp"

class ScreenshotHandler {
private:
#ifdef __SWITCH__
	Result rc;
#endif

public:
	ScreenshotHandler();

	// Returns a dHash (to determine similarity) and a jpegBuffer to actually view
	void writeFramebuffer(std::shared_ptr<CommunicateWithNetwork> networkInstance, uint8_t linkedWithFrameAdvance, uint32_t frame, uint16_t savestateHookNum, uint8_t playerIndex);

	~ScreenshotHandler();
};