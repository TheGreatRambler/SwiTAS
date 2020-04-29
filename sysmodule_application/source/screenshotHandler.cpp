#include "screenshotHandler.hpp"

ScreenshotHandler::ScreenshotHandler() {
	jpegBuf = (uint8_t*)malloc(JPEG_BUF_SIZE);
}

void ScreenshotHandler::writeFramebuffer(std::shared_ptr<CommunicateWithNetwork> networkInstance) {
	uint64_t outSize;
	rc = capsscCaptureJpegScreenShot(&outSize, jpegBuf, JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, 100000000);
	LOGD << "Done capturing screenshot";
	if(R_SUCCEEDED(rc)) {
		std::vector<uint8_t> buf(outSize);
		memcpy(buf.data(), jpegBuf, outSize);
		// clang-format off
		ADD_TO_QUEUE(RecieveGameFramebuffer, networkInstance, {
			data.buf   = buf;
		})
		// clang-format on
	}
}

ScreenshotHandler::~ScreenshotHandler() {
	free(jpegBuf);
}
