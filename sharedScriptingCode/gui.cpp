#include "gui.hpp"

Gui::Gui(int screenWidth, int screenHeight) {
	width  = screenWidth;
	height = screenHeight;

#ifdef __SWITCH__
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
	rc = framebufferCreate(&framebuf, &window, screenWidth, screenHeight, PIXEL_FORMAT_RGBA_4444, 1);
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

	static PlFontData stdFontData, extFontData;

	// Nintendo's default font
	R_TRY(plGetSharedFontByType(&stdFontData, PlSharedFontType_Standard));

	u8* fontBuffer = reinterpret_cast<u8*>(stdFontData.address);
	stbtt_InitFont(&stdNintendoFont, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));

	// Nintendo's extended font containing a bunch of icons
	R_TRY(plGetSharedFontByType(&extFontData, PlSharedFontType_NintendoExt));

	fontBuffer = reinterpret_cast<u8*>(extFontData.address);
	stbtt_InitFont(&extNintendoFont, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));
#endif
}

void Gui::startFrame() {
// Dequeue
#ifdef __SWITCH__
	currentBuffer = (uint8_t*)framebufferBegin(&framebuf, nullptr);
#endif
}

void Gui::endFrame() {
// Flush
#ifdef __SWITCH__
	framebufferEnd(&framebuf);
#endif
}

void Gui::setPixel(uint32_t x, uint32_t y, Color color) {
// Outstride is 4 and bytes per pixel is 4
#ifdef __SWITCH__
	uint32_t offset                 = getPixelOffset(x, y);
	((Color*)currentBuffer)[offset] = color;
#endif
}

void Gui::takeScreenshot(std::string path) {
#ifdef __SWITCH__
	uint64_t outSize;
	std::vector<uint8_t> buf(JPEG_BUF_SIZE);
	rc = capsscCaptureJpegScreenShot(&outSize, buf.data(), JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, 100000000);
	if(R_SUCCEEDED(rc)) {
		FILE* jpegFile = fopen(path.c_str(), “wb +”);
		fwrite(buf.data(), outSize, 1, jpegFile);
		fclose(jpegFile);
	}
#endif
}

Gui::~Gui() {
// Close everything
#ifdef __SWITCH__
	framebufferClose(&framebuf);
	nwindowClose(&window);
	viDestroyManagedLayer(&layer);
	viCloseDisplay(&display);
	viExit();
#endif
}