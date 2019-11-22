#pragma once

extern "C" {
#include <switch.h>
}

// I dunno what this is
extern "C" u64 __nx_vi_layer_id;

class WriteToScreen() {
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
	WriteToScreen(int screenWidth, int screenHeight) {
		width = screenWidth;
		height = screenHeight;
		// https://github.com/averne/dvdnx/blob/master/src/screen.cpp
		rc = smInitialize();
		if (R_FAILED(rc)) {
			fatalSimple(rc);
		}
		// ViServiceType_Manager defined here https://switchbrew.github.io/libnx/vi_8h.html
		rc = viInitialize(ViServiceType_Manager);
		if (R_FAILED(rc)) {
			fatalSimple(rc);
		}
		rc = viOpenDefaultDisplay(&display);
		if (R_FAILED(rc)) {
			viExit();
			return;
		}
		// flag 0 allows non-fullscreen layer
		rc = viCreateManagedLayer(&display, (ViLayerFlags) 0, 0, &__nx_vi_layer_id);
		if (R_FAILED(rc)) {
			viCloseDisplay(&display);
			return;
		}
		rc = viCreateLayer(&display, &layer);
		if (R_FAILED(rc)) {
			viDestroyManagedLayer(&layer);
			return;
		}
		rc = viSetLayerScalingMode(&layer, ViScalingMode_FitToLayer);
		if (R_FAILED(rc)) {
			viDestroyManagedLayer(&layer);
			return;
		}
		// Arbitrary z index
		rc = viSetLayerZ(&layer, 100);
		if (R_FAILED(rc)) {
			viDestroyManagedLayer(&layer);
			return;
		}
		// These might not be screenWidth and screenHeight TODO
		// They are smaller in the source
		rc = viSetLayerSize(&layer, screenWidth, screenHeight);
		if (R_FAILED(rc)) {
			viDestroyManagedLayer(&layer);
			return;
		}
		// The X and Y positions of the layer
		rc = viSetLayerPosition(&layer, 0.0f, 0.0f);
		if (R_FAILED(rc)) {
			viDestroyManagedLayer(&layer);
			return;
		}
		rc = nwindowCreateFromLayer(&window, &layer);
		if (R_FAILED(rc)) {
			viDestroyManagedLayer(&layer);
			return;
		}
		// PIXEL_FORMAT_RGBA_8888  defined in LibNX (based on Android)
		// Not PIXEL_FORMAT_RGBA_8888 in source, but I don't care
		// Upscaling and downscaling will happen so that the Layer Size and the FB size match
		rc = framebufferCreate(&framebuf, &window, screenWidth, screenHeight, PIXEL_FORMAT_RGBA_8888, 1);
		if (R_FAILED(rc)) {
			nwindowClose(&window);
			return;
		}

		// Make Framebuffer linear to make things easier
		// Imma too dumb to figure out the raw format
		// 4 bytes per pixel (outstride)
		rc = framebufferMakeLinear(&framebuf, 4);
		if (R_FAILED(rc)) {
			nwindowClose(&window);
			return;
		}

		rc = smExit();
		if (R_FAILED(rc)) {
			fatalSimple(rc);
		}
	}

	void startFrame() {
		// Dequeue
		currentBuffer = framebufferBegin(&framebuf, NULL);
	}

	void endFrame() {
		// Flush
		framebufferEnd(&framebuf);
	}

	void setPixel(u32 x, u32 y, u8 red, u8 green, u8 blue) {
		// Outstride is 4 and bytes per pixel is 4
		u32 pixelOffset = y * width * 4 + x * 4;
		currentBuffer[pixelOffset] = red;
		currentBuffer[pixelOffset + 1] = green;
		currentBuffer[pixelOffset + 2] = blue;
	}

	void makeOpaque() {
		// Make entire framebuffer opaque
		u8 size = width * height;
		for (int i = 3; i < size; i += 4) {
			// Set the opacity byte to opaque
			currentBuffer[i] = 0xFF;
		}
	}

	void makeClear() {
		// Make entire framebuffer opaque
		u8 size = width * height;
		for (int i = 3; i < size; i += 4) {
			// Set the opacity byte to opaque
			currentBuffer[i] = 0x00;
		}
	}

	~WriteToScreen() {
		// Close everything
		framebufferClose(&framebuf);
		nwindowClose(&window);
		viDestroyManagedLayer(&layer);
		viCloseDisplay(&display);
		viExit();
	}
}