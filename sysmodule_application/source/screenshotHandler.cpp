#include "screenshotHandler.hpp"

ScreenshotHandler::ScreenshotHandler() {}

void ScreenshotHandler::writeFramebuffer(std::shared_ptr<CommunicateWithNetwork> networkInstance) {
	uint64_t outSize;
	std::vector<uint8_t> buf(JPEG_BUF_SIZE);
	rc = capsscCaptureJpegScreenShot(&outSize, buf->data(), JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, 100000000);
	if(R_SUCCEEDED(rc)) {
		LOGD << "Screenshot size: " << outSize;
		buf.resize(outSize);
		// clang-format off
		ADD_TO_QUEUE(RecieveGameFramebuffer, networkInstance, {
			data.buf   = buf;
		})
		// clang-format on
	}
}

ScreenshotHandler::~ScreenshotHandler() {}
