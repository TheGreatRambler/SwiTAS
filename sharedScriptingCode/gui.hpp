#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

#define JPEG_BUF_SIZE 0x80000
#define STB_TRUETYPE_IMPLEMENTATION

#include "include/stb_truetype.h"

#ifdef __SWITCH__
extern "C" u64 __nx_vi_layer_id;
#endif

// Libtesla color
// RGBA4444
// Even on yuzu, this is the same
struct Color {
	uint16_t r : 4, g : 4, b : 4, a : 4;
} __attribute__((packed));

class Gui {
private:
#ifdef __SWITCH__
	ViDisplay display;
	ViLayer layer;
	NWindow window;
	Framebuffer framebuf;
	// Error handling for everything
	Result rc;
	// Current pointer to the graphics data
	uint8_t* currentBuffer;
#endif
	// Dimensions
	// Always 1280x720
	int width;
	int height;

#ifdef __SWITCH__
	stbtt_fontinfo stdNintendoFont;
	stbtt_fontinfo extNintendoFont;
#else
	stbtt_fontinfo stdFont;
#endif

#ifdef __SWITCH__
	u32 getPixelOffset(s32 x, s32 y) {
		u32 tmpPos = ((y & 127) / 16) + (x / 32 * 8) + ((y / 16 / 8) * (((width / 2) / 16 * 8)));
		tmpPos *= 16 * 16 * 4;

		tmpPos += ((y % 16) / 8) * 512 + ((x % 32) / 16) * 256 + ((y % 8) / 2) * 64 + ((x % 16) / 8) * 32 + (y % 2) * 16 + (x % 8) * 2;

		return tmpPos / 2;
	}
#endif

public:
	Gui(int screenWidth, int screenHeight);

	void startFrame();
	void endFrame();

	void setPixel(uint32_t x, uint32_t y, Color color);

	void takeScreenshot(std::string path);

	~Gui();
};