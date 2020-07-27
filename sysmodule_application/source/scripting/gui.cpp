#include "gui.hpp"

Gui::Gui() {

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
	rc = viSetLayerSize(&layer, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
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
	rc = framebufferCreate(&framebuf, &window, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, PIXEL_FORMAT_RGBA_4444, 1);
	if(R_FAILED(rc)) {
		nwindowClose(&window);
		fatalThrow(rc);
	}

	// Make Framebuffer linear to make things easier
	// Imma too dumb to figure out the raw format
	// 4 bytes per pixel (outstride)
	// rc = framebufferMakeLinear(&framebuf);
	// if(R_FAILED(rc)) {
	//	nwindowClose(&window);
	//	fatalThrow(rc);
	//}

	static PlFontData stdFontData, extFontData;

	// Nintendo's default font
	rc = plGetSharedFontByType(&stdFontData, PlSharedFontType_Standard);

	if(R_FAILED(rc)) {
		nwindowClose(&window);
		fatalThrow(rc);
	}

	u8* fontBuffer = reinterpret_cast<u8*>(stdFontData.address);
	stbtt_InitFont(&stdNintendoFont, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));

	// Nintendo's extended font containing a bunch of icons
	rc = plGetSharedFontByType(&extFontData, PlSharedFontType_NintendoExt);

	if(R_FAILED(rc)) {
		nwindowClose(&window);
		fatalThrow(rc);
	}

	fontBuffer = reinterpret_cast<u8*>(extFontData.address);
	stbtt_InitFont(&extNintendoFont, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));

	savedJpegFramebuffer = (uint8_t*)malloc(JPEG_BUF_SIZE);
#endif

	fbg = fbg_customSetup(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 4, false, false, (void*)this, Gui::framebufferDraw, NULL, NULL, NULL);
	if(!fbg) {
		// return NULL;
	}

	fbg->back_buffer = (uint8_t*)calloc(1, fbg->size * sizeof(uint8_t));
	if(!fbg->back_buffer) {
		// return NULL;
	}

#ifdef __SWITCH__
	std::string rootImagePath = controllerOverlayDirectory + "/";
#endif

	// Set every button to its image
	for(auto const& imageName : btnOverlayImageNames) {
		controllerImages[imageName.first] = fbg_loadPNG(fbg, (rootImagePath + imageName.second).c_str());
	}

	blankControllerImage = fbg_loadPNG(fbg, (rootImagePath + "blank.png").c_str());
	leftStickImage       = fbg_loadPNG(fbg, (rootImagePath + "leftstick.png").c_str());
	rightStickImage      = fbg_loadPNG(fbg, (rootImagePath + "rightstick.png").c_str());
}

void Gui::startFrame() {
// Dequeue
#ifdef __SWITCH__
	currentBuffer = (uint8_t*)framebufferBegin(&framebuf, nullptr);
#endif
	fbg_clear(fbg, 0);
	fbg_draw(fbg);
}

void Gui::endFrame() {
	// Flush
	fbg_flip(fbg);
#ifdef __SWITCH__
	framebufferEnd(&framebuf);
#endif
}

void Gui::setPixel(uint32_t x, uint32_t y, _fbg_rgb color) {
	fbg_pixela(fbg, x, y, color.r, color.g, color.b, color.a);
}

void Gui::framebufferDraw(struct _fbg* fbg) {
	Gui* gui = (Gui*)fbg->user_context;
#ifdef __SWITCH__
	Color* buf = (Color*)gui->currentBuffer;
#endif

	int x, y;
	for(x = 0; x < FRAMEBUFFER_WIDTH; ++x) {
		for(y = 0; y < FRAMEBUFFER_WIDTH; ++y) {
			int index = x + y * FRAMEBUFFER_WIDTH;

			Color color;

			color.r = fbg->back_buffer[index * fbg->components];
			color.g = fbg->back_buffer[index * fbg->components + 1];
			color.b = fbg->back_buffer[index * fbg->components + 2];
			color.a = fbg->back_buffer[index * fbg->components + 3];
			;

#ifdef __SWITCH__
			buf[getPixelOffset(x, y)] = color;
#endif
		}
	}
}

void Gui::drawText(uint32_t x, uint32_t y, float size, std::string text) {
	// Color not passed, need to use fbg_fill() beforehand
	const char* string = text.c_str();
	uint32_t currentX  = x;
	uint32_t currentY  = y;

	do {
		uint32_t currentCharacter;
		ssize_t codepointWidth = decode_utf8(&currentCharacter, (uint8_t*)string);

		if(codepointWidth <= 0)
			break;

		string += codepointWidth;

		stbtt_fontinfo* currentFont = fontForGlyph(currentCharacter);
		float currentFontSize       = stbtt_ScaleForPixelHeight(currentFont, size);

		if(currentCharacter == '\n') {
			currentX = x;
			currentY += (uint32_t)size;

			continue;
		}

		int bounds[4] = { 0 };
		stbtt_GetCodepointBitmapBoxSubpixel(currentFont, currentCharacter, currentFontSize, currentFontSize, 0, 0, &bounds[0], &bounds[1], &bounds[2], &bounds[3]);

		int32_t x = 0, y = 0;
		stbtt_GetCodepointHMetrics(currentFont, currentCharacter, &x, &y);

		if(!std::iswspace(currentCharacter)) {
			int width, height;

			u8* glyphBmp = stbtt_GetCodepointBitmap(currentFont, size, size, currentCharacter, &width, &height, nullptr, nullptr);

			if(glyphBmp == nullptr)
				return;

			for(u32 bmpY = 0; bmpY < height; bmpY++) {
				for(u32 bmpX = 0; bmpX < width; bmpX++) {
					_fbg_rgb tmpColor;

					tmpColor.r = currentColor.r;
					tmpColor.g = currentColor.g;
					tmpColor.b = currentColor.b;
					tmpColor.a = currentColor.a;

					tmpColor.a = (glyphBmp[width * bmpY + bmpX] >> 4) * (float(tmpColor.a) / 0xF);

					fbg_pixela(fbg, currentX + bounds[0] + bmpX, currentY + bounds[1] + bmpY, tmpColor.r, tmpColor.g, tmpColor.b, tmpColor.a);
				}
			}

			free(glyphBmp);
		}

		currentX += static_cast<u32>(x * currentFontSize);
	} while(*string != '\0');
}

void Gui::drawControllerOverlay(HiddbgHdlsState& state, float scale, uint32_t x, uint32_t y) {
	fbg_imageScale(fbg, blankControllerImage, x, y, scale, scale);

	for(auto const& button : btnToHidKeys) {
#ifdef __SWITCH__
		if(state.buttons & button.second) {
			fbg_imageScale(fbg, controllerImages[button.first], x, y, scale, scale);
		}
#endif
	}

	int32_t deltaXLeft = (state.joysticks[JOYSTICK_LEFT].dx / joystickRangeConstant) * scale;
	int32_t deltaYLeft = (state.joysticks[JOYSTICK_LEFT].dy / joystickRangeConstant) * scale;
	fbg_imageScale(fbg, leftStickImage, x + deltaXLeft, y + deltaYLeft, scale, scale);

	int32_t deltaXRight = (state.joysticks[JOYSTICK_RIGHT].dx / joystickRangeConstant) * scale;
	int32_t deltaYRight = (state.joysticks[JOYSTICK_RIGHT].dy / joystickRangeConstant) * scale;
	fbg_imageScale(fbg, rightStickImage, x + deltaXRight, y + deltaYRight, scale, scale);
}

void Gui::drawControllerOverlay(uint8_t playerIndex, HiddbgHdlsState& state) {
	drawControllerOverlay(state, 1.0f, playerIndex * 160, 720 - 120);
}

void Gui::takeScreenshot(std::string path) {
#ifdef __SWITCH__
	uint64_t outSize;
	rc = capsscCaptureJpegScreenShot(&outSize, savedJpegFramebuffer, JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, 100000000);
	if(R_SUCCEEDED(rc)) {
		FILE* jpegFile = fopen(path.c_str(), "wb+");
		fwrite(savedJpegFramebuffer, outSize, 1, jpegFile);
		fclose(jpegFile);
	}
#endif
}

Gui::~Gui() {
	// Close everything
	for(auto& image : controllerImages) {
		fbg_freeImage(image.second);
	}

	fbg_freeImage(blankControllerImage);
	fbg_freeImage(leftStickImage);
	fbg_freeImage(rightStickImage);

	fbg_close(fbg);

#ifdef __SWITCH__
	free(savedJpegFramebuffer);

	framebufferClose(&framebuf);
	nwindowClose(&window);
	viDestroyManagedLayer(&layer);
	viCloseDisplay(&display);
	viExit();
#endif
}