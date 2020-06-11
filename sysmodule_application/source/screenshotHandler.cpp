#include "screenshotHandler.hpp"

ScreenshotHandler::ScreenshotHandler() {}

void ScreenshotHandler::writeFramebuffer(std::shared_ptr<CommunicateWithNetwork> networkInstance, uint8_t linkedWithFrameAdvance, uint32_t frame, uint16_t savestateHookNum, uint8_t playerIndex) {
#ifdef __SWITCH__
	LOGD << "Linked with frame advance: " << (int)linkedWithFrameAdvance;
#endif
	uint64_t outSize;
	std::vector<uint8_t> buf(JPEG_BUF_SIZE);

#ifdef __SWITCH__
	rc                = capsscCaptureJpegScreenShot(&outSize, buf.data(), JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, 100000000);
	uint8_t succeeded = R_SUCCEEDED(rc);
#else
	uint8_t succeeded = true;
#endif

	if(succeeded) {
#ifdef __SWITCH__
		LOGD << "Screenshot size: " << outSize;
#endif
		buf.resize(outSize);
		// clang-format off
		ADD_TO_QUEUE(RecieveGameFramebuffer, networkInstance, {
			data.buf   = buf;
			data.fromFrameAdvance = linkedWithFrameAdvance;
			data.frame = frame;
			data.savestateHookNum = savestateHookNum;
			data.playerIndex = playerIndex;
		})
		// clang-format on
	}
	// Technically this can fail, TODO handle that case
}

ScreenshotHandler::~ScreenshotHandler() {}
