#pragma once

#include <switch.h>

// I dunno what this is
extern "C" u64 __nx_vi_layer_id;

class WriteToScreen {
private:
	ViDisplay display;
	ViLayer layer;
	NWindow window;
	Framebuffer framebuf;
	// Error handling for everything
	Result rc;
	// Current pointer to the graphics data
	u8* currentBuffer;
	// Dimensions
	int width;
	int height;

public:
	WriteToScreen(int screenWidth, int screenHeight);

	void startFrame();

	void endFrame();

	void setPixel(u32 x, u32 y, u8 red, u8 green, u8 blue);

	void makeOpaque();

	void makeClear();

	~WriteToScreen();
};