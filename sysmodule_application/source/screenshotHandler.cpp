#include "screenshotHandler.hpp"

ScreenshotHandler::ScreenshotHandler() {}

void ScreenshotHandler::writeFramebuffer(std::vector<uint8_t>& buf) {
	uint64_t outSize;

	uint8_t succeeded = true;

#ifdef __SWITCH__
	rc        = capsscCaptureJpegScreenShot(&outSize, buf.data(), JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, INT64_MAX);
	succeeded = R_SUCCEEDED(rc);
#else
	succeeded = true;
#endif

	if(succeeded) {
		buf.resize(outSize);
	}
	// Technically this can fail, TODO handle that case

#ifdef __SWITCH__
	capsscOpenRawScreenShotReadStream(&outSize, &outSize, &outSize, ViLayerStack::ViLayerStack_ApplicationForDebug, INT64_MAX);

	// Calculate dhash
	// Resize width to 16
	// Resize height to 15
	uint8_t leftPixel;
	uint8_t rightPixel;
	// For loops for resized pixels
	for(uint16_t y = 0; y < 15; y++) {
		for(uint16_t x = 1; x < 16; x++) {
			uint8_t tempPixelBuf[(1280 / 16) * 4];
			uint32_t allPixelsCombined = 0;

			for(uint8_t subRow = 0; subRow < 720 / 15; subRow++) {
				// Only read the RGB portions
				// TODO set up math for this part
				readFullScreenshotStream(tempPixelBuf, sizeof(tempPixelBuf), y * (720 / 15) * 1280 * 4);
			}
		}
	}
#else
	succeeded = true;
#endif
}

#ifdef __SWITCH__
void ScreenshotHandler::readFullScreenshotStream(uint8_t* buf, uint64_t size, uint64 offset) {
	uint64_t sizeActuallyRead = 0;

	while(sizeActuallyRead != size) {
		uint64_t bytesRead;
		capsscReadRawScreenShotReadStream(&bytesRead, &buf[sizeActuallyRead], size - sizeActuallyRead, offset + sizeActuallyRead);
		sizeActuallyRead += bytesRead;
	}
}
#endif

ScreenshotHandler::~ScreenshotHandler() {}
