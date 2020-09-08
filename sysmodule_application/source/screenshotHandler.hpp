#pragma once

#define JPEG_BUF_SIZE 0x80000

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#ifdef __SWITCH__
#include <plog/Log.h>
#include <switch.h>
#endif

#ifdef YUZU
#include "yuzuSyscalls.hpp"
#endif

class ScreenshotHandler {
private:
	const uint8_t dhashWidth  = 80;
	const uint8_t dhashHeight = 45;

#ifdef YUZU
	std::shared_ptr<Syscalls> yuzuInstance;
#endif

#ifdef __SWITCH__
	Result rc;
#endif

#ifdef __SWITCH__
	void readFullScreenshotStream(uint8_t* buf, uint64_t size, uint64_t offset);
#endif

public:
	ScreenshotHandler();

#ifdef YUZU
	void setYuzuInstance(std::shared_ptr<Syscalls> yuzu) {
		yuzuInstance = yuzu;
	}
#endif

	void writeFramebuffer(std::vector<uint8_t>& buf, std::string& dhash);

	~ScreenshotHandler();
};