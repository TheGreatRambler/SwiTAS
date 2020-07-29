#include "screenshotHandler.hpp"

ScreenshotHandler::ScreenshotHandler() {}

void ScreenshotHandler::writeFramebuffer(std::vector<uint8_t>& buf, std::string& dhash) {
	buf.resize(JPEG_BUF_SIZE);
	uint64_t outSize;
	uint8_t succeeded = true;

#ifdef __SWITCH__
	rc        = capsscCaptureJpegScreenShot(&outSize, buf.data(), JPEG_BUF_SIZE, ViLayerStack::ViLayerStack_ApplicationForDebug, INT64_MAX);
	succeeded = R_SUCCEEDED(rc);
#else
	uint8_t* jpeg = yuzuSyscalls->function_emu_getscreenjpeg(yuzuSyscalls->getYuzuInstance(), &outSize);
	memcpy(buf.data(), jpeg, outSize);
	free(jpeg);
	succeeded = true;
#endif

	if(succeeded) {
		buf.resize(outSize);
	}
	// Technically this can fail, TODO handle that case
	/*

#ifdef __SWITCH__
	capsscOpenRawScreenShotReadStream(&outSize, &outSize, &outSize, ViLayerStack::ViLayerStack_ApplicationForDebug, INT64_MAX);

	const int dhashChunkWidth  = (1280 / dhashWidth);
	const int dhashChunkHeight = (720 / dhashHeight);

	// Calculate dhash
	// Resize width to 16
	// Resize height to 15
	double leftPixel;
	double rightPixel;
	uint8_t haveSetRightPixel = false;
	char hash[dhashChunkWidth * (dhashChunkHeight - 1)];
	// For loops for resized pixels
	for(uint16_t y = 0; y < dhashHeight; y++) {
		for(uint16_t x = 0; x < dhashWidth; x++) {
			uint8_t tempPixelBuf[dhashChunkWidth * 4];
			uint32_t allPixelsCombined = 0;

			for(uint8_t subRow = 0; subRow < dhashChunkHeight; subRow++) {
				// Only read the RGB portions
				uint64_t offset = (y * dhashChunkHeight) * (1280 * 4) + subRow * (1280 * 4) + (x * dhashChunkWidth * 4);
				readFullScreenshotStream(tempPixelBuf, sizeof(tempPixelBuf), offset);

				for(uint8_t i = 0; i < dhashChunkWidth; i++) {
					allPixelsCombined += tempPixelBuf[i * 4];
					allPixelsCombined += tempPixelBuf[i * 4 + 1];
					allPixelsCombined += tempPixelBuf[i * 4 + 2];
				}
			}

			double average = (double)allPixelsCombined / (dhashChunkHeight * (dhashChunkWidth * 3));
			if(!haveSetRightPixel) {
				// First pixel. set left
				leftPixel         = average;
				haveSetRightPixel = true;
			} else {
				// Set right pixel, compare to previous, then move
				rightPixel = average;
				if(leftPixel > rightPixel) {
					hash[x - 1] = '1';
				} else {
					hash[x - 1] = '0';
				}
				leftPixel = rightPixel;
			}
		}
	}

	capsscCloseRawScreenShotReadStream();

	dhash.assign(hash, sizeof(hash));
#else
	succeeded = true;
#endif
*/
}

#ifdef __SWITCH__
void ScreenshotHandler::readFullScreenshotStream(uint8_t* buf, uint64_t size, uint64_t offset) {
	uint64_t sizeActuallyRead = 0;

	while(sizeActuallyRead != size) {
		uint64_t bytesRead;
		capsscReadRawScreenShotReadStream(&bytesRead, &buf[sizeActuallyRead], size - sizeActuallyRead, offset + sizeActuallyRead);
		sizeActuallyRead += bytesRead;
	}
}
#endif

ScreenshotHandler::~ScreenshotHandler() {}
