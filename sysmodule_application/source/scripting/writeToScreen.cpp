#include "writeToScreen.hpp"

WriteToScreen::WriteToScreen(int screenWidth, int screenHeight) {
	width  = screenWidth;
	height = screenHeight;
	// https://github.com/averne/dvdnx/blob/master/src/screen.cpp
	rc = smInitialize();
	if(R_FAILED(rc)) {
		fatalThrow(rc);
	}
	// ViServiceType_Manager defined here https://switchbrew.github.io/libnx/vi_8h.html
	rc = viInitialize(ViServiceType_Manager);
	if(R_FAILED(rc)) {
		fatalThrow(rc);
	}
	rc = viOpenDefaultDisplay(&display);
	if(R_FAILED(rc)) {
		viExit();
		return;
	}
	// flag 0 allows non-fullscreen layer
	rc = viCreateManagedLayer(&display, (ViLayerFlags)0, 0, &__nx_vi_layer_id);
	if(R_FAILED(rc)) {
		viCloseDisplay(&display);
		fatalThrow(rc);
	}
	rc = viCreateLayer(&display, &layer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	rc = viSetLayerScalingMode(&layer, ViScalingMode_FitToLayer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// Arbitrary z index
	rc = viSetLayerZ(&layer, 100);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// These might not be screenWidth and screenHeight TODO
	// They are smaller in the source
	rc = viSetLayerSize(&layer, screenWidth, screenHeight);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// The X and Y positions of the layer
	rc = viSetLayerPosition(&layer, 0.0f, 0.0f);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	rc = nwindowCreateFromLayer(&window, &layer);
	if(R_FAILED(rc)) {
		viDestroyManagedLayer(&layer);
		fatalThrow(rc);
	}
	// PIXEL_FORMAT_RGBA_8888  defined in LibNX (based on Android)
	// Not PIXEL_FORMAT_RGBA_8888 in source, but I don't care
	// Upscaling and downscaling will happen so that the Layer Size and the FB size match
	rc = framebufferCreate(&framebuf, &window, screenWidth, screenHeight, PIXEL_FORMAT_RGBA_8888, 1);
	if(R_FAILED(rc)) {
		nwindowClose(&window);
		fatalThrow(rc);
	}

	// Make Framebuffer linear to make things easier
	// Imma too dumb to figure out the raw format
	// 4 bytes per pixel (outstride)
	rc = framebufferMakeLinear(&framebuf);
	if(R_FAILED(rc)) {
		nwindowClose(&window);
		fatalThrow(rc);
	}
}

void WriteToScreen::startFrame() {
	// Dequeue
	currentBuffer = (u8*)framebufferBegin(&framebuf, NULL);
}

void WriteToScreen::endFrame() {
	// Flush
	framebufferEnd(&framebuf);
}

void WriteToScreen::setPixel(u32 x, u32 y, u8 red, u8 green, u8 blue) {
	// Outstride is 4 and bytes per pixel is 4
	u32 pixelOffset                = y * width * 4 + x * 4;
	currentBuffer[pixelOffset]     = red;
	currentBuffer[pixelOffset + 1] = green;
	currentBuffer[pixelOffset + 2] = blue;
}

void WriteToScreen::makeOpaque() {
	// Make entire framebuffer opaque
	u8 size = width * height;
	for(int i = 3; i < size; i += 4) {
		// Set the opacity byte to opaque
		currentBuffer[i] = 0xFF;
	}
}

void WriteToScreen::makeClear() {
	// Make entire framebuffer opaque
	u8 size = width * height;
	for(int i = 3; i < size; i += 4) {
		// Set the opacity byte to opaque
		currentBuffer[i] = 0x00;
	}
}

WriteToScreen::~WriteToScreen() {
	// Close everything
	framebufferClose(&framebuf);
	nwindowClose(&window);
	viDestroyManagedLayer(&layer);
	viCloseDisplay(&display);
	viExit();
}