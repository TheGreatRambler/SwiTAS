#include "screenshotHandler.hpp"

ScreenshotHandler::ScreenshotHandler() {}

void ScreenshotHandler::writeFramebuffer(std::vector<uint8_t>& buf) {
	uint64_t outSize;

#ifdef __SWITCH__
	rc                = capsscCaptureJpegScreenShot(&outSize, buf.data(), JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, 100000000);
	uint8_t succeeded = R_SUCCEEDED(rc);
#else
	uint8_t succeeded = true;
#endif

	if(succeeded) {
		buf.resize(outSize);
	}
	// Technically this can fail, TODO handle that case
}

ScreenshotHandler::~ScreenshotHandler() {}
