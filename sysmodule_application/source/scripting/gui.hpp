#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

#define JPEG_BUF_SIZE 0x80000
#define STB_TRUETYPE_IMPLEMENTATION

#include <stb_truetype.h>

#ifdef __SWITCH__
extern "C" u64 __nx_vi_layer_id;
#endif

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720

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
	uint8_t* savedJpegFramebuffer;
#endif

#ifdef __SWITCH__
	stbtt_fontinfo stdNintendoFont;
	stbtt_fontinfo extNintendoFont;
#else
	stbtt_fontinfo stdFont;
#endif

#ifdef __SWITCH__
	inline u32 getPixelOffset(u32 x, u32 y) {
		// Swizzling pattern:
		//    y6,y5,y4,y3,y2,y1,y0,x7,x6,x5,x4,x3,x2,x1,x0
		// -> x7,x6,x5,y6,y5,y4,y3,x4,y2,y1,x3,y0,x2,x1,x0
		// Bits x0-4 and y0-6 are from memory layout spec (see TRM 20.1.2 - Block Linear) and libnx hardcoded values
		constexpr u32 x_mask = (__builtin_ctz(FRAMEBUFFER_WIDTH) - 1) << 5;
		const u32 swizzled_x = ((x & x_mask) * 128) + ((x & 0b00010000) * 8) + ((x & 0b00001000) * 2) + (x & 0b00000111);
		const u32 swizzled_y = ((y & 0b1111000) * 32) + ((y & 0b0000110) * 16) + ((y & 0b0000001) * 8);
		return swizzled_x + swizzled_y;
	}
#endif

public:
	Gui();

	void startFrame();
	void endFrame();

	void setPixel(uint32_t x, uint32_t y, Color color);

	void takeScreenshot(std::string path);

	~Gui();
};