#include "screenshotHandler.hpp"

ScreenshotHandler::ScreenshotHandler() {
	// Get the PID from the Title ID
	pmdmntGetProcessId(&VI_pid, VITitleId);
	// Get the pointer to the main framebuffer in VI
	rc = svcDebugActiveProcess(&VIdbg, VI_pid);
	if(R_SUCCEEDED(rc)) {
		uint64_t addr = 0;
		// Loops for 0x1000 because it shouldn't go infinitely
		for(int i = 0; i < 0x1000; i++) {
			MemoryInfo info;
			uint32_t pageinfo;
			rc = svcQueryDebugProcessMemory(&info, &pageinfo, VIdbg, addr);
			if(info.type == MemType_CodeMutable) {
				if(info.size == 0x1A40000) {
					// Set the address now that it is known
					framebufferPointer = info.addr;
					LOGD << "Framebuffer pointer found: " << framebufferPointer;
					break;
				}
			} else if(info.type == MemType_Reserved) {
				// Apparently getting into reserved memory means we have gone too far
				break;
			}
			addr += info.size;
		}
		svcCloseHandle(VIdbg);
	}
	row_pointer[0] = (uint8_t*)malloc(jpegFramebufferScanlineSize);
}

void ScreenshotHandler::writeFramebuffer(std::string* hash, std::vector<uint8_t>* jpegBuffer) {
	// Encode the file with libjpeg
	// https://www.ridgesolutions.ie/index.php/2019/12/10/libjpeg-example-encode-jpeg-to-memory-buffer-instead-of-file/
	// https://www.geeksforgeeks.org/hamming-distance-between-two-integers/
	// https://github.com/cascornelissen/dhash-image/blob/master/index.js
	// https://tannerhelland.com/2011/10/01/grayscale-image-algorithm-vb6.html
	// https://raw.githubusercontent.com/libjpeg-turbo/libjpeg-turbo/master/libjpeg.txt
	// https://dev.w3.org/Amaya/libjpeg/example.c
	// https://github.com/LuaDist/libjpeg/blob/master/example.c
	/*

	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err       = jpeg_std_error(&jerr);
	jerr.error_exit = [](j_common_ptr cinfo) {
		// Allow it to exit
		(*cinfo->err->output_message)(cinfo);
		jpeg_destroy(cinfo);
		exit(EXIT_FAILURE);
	};
	jerr.output_message = [](j_common_ptr cinfo) {
		char pszErr[JMSG_LENGTH_MAX];
		(*cinfo->err->format_message)(cinfo, pszErr);
		std::string error(pszErr, sizeof(pszErr));
		LOGD << "LIBJPEG WARNING: " << error;
	};

	jpeg_create_compress(&cinfo);

	// Encode to memory
	FILE* outfile = fopen(tempScreenshotName, "wb");
	// unsigned char* jpegBuf;
	// unsigned long jpegSize;
	// jpeg_mem_dest(&cinfo, &jpegBuf, &jpegSize);
	jpeg_stdio_dest(&cinfo, outfile);

	LOGD << "Create mem dest";

	cinfo.image_width      = framebufferWidth;
	cinfo.image_height     = framebufferHeight;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, jpegQuality, true);

	LOGD << "Set defaults: " << framebufferWidth << " " << framebufferHeight;

	// CRASHES RIGHT HERE
	jpeg_start_compress(&cinfo, true);

	LOGD << "Start compress";

	// Can add a comment section too :)
	const char* comment = "JPEG generated by nx-TAS";
	jpeg_write_marker(&cinfo, JPEG_COM, (const JOCTET*)comment, strlen(comment));

	LOGD << "Write comment marker";

	uint64_t initialPointer = framebufferPointer + FramebufferType::HOME1 * framebufferSize;
	// Encode

	uint8_t dhash[sizeOfDhash];
	int indexByteDhash    = 0;
	uint8_t indexBitDhash = 0;
	// Might need to use this, but would possibly be more inefficent
	// svcMapProcessMemory
	// Debug VI before anything
	rc = svcDebugActiveProcess(&VIdbg, VI_pid);
	if(R_SUCCEEDED(rc)) {
		LOGD << "Debugging VI worked";
		while(cinfo.next_scanline < cinfo.image_height) {
			// Obtain data for each row
			for(int yOffset = 0; yOffset < heightOfdhashInput; yOffset++) {
				for(int x = 0; x < framebufferWidth; x++) {
					uint16_t y = cinfo.next_scanline + yOffset;
					// Get the index
					// https://github.com/averne/dvdnx/blob/master/src/screen.hpp#L74
					// Swizzling pattern:
					//    y6,y5,y4,y3,y2,y1,y0,x7,x6,x5,x4,x3,x2,x1,x0
					// -> x7,x6,x5,y6,y5,y4,y3,x4,y2,y1,x3,y0,x2,x1,x0
					// Bits x0-4 and y0-6 are from memory layout spec (see TRM 20.1.2 - Block Linear) and libnx hardcoded values
					constexpr uint32_t x_mask = (__builtin_ctz(framebufferWidth) - 1) << 5;
					const uint32_t swizzled_x = ((x & x_mask) * 128) + ((x & 0b00010000) * 8) + ((x & 0b00001000) * 2) + (x & 0b00000111);
					const uint32_t swizzled_y = ((y & 0b1111000) * 32) + ((y & 0b0000110) * 16) + ((y & 0b0000001) * 8);
					uint32_t index            = swizzled_x + swizzled_y;

					// Laid out like RGBA
					uint8_t colorParts[4];
					svcReadDebugProcessMemory(&colorParts, VIdbg, initialPointer + index, sizeof(colorParts));

					// Set each value into the color data for this section of the JPEG
					uint16_t startDataIndex = x * jpegBytesPerPixel;

					uint8_t red   = colorParts[0];
					uint8_t green = colorParts[1];
					uint8_t blue  = colorParts[2];

					row_pointer[yOffset][startDataIndex]     = red;
					row_pointer[yOffset][startDataIndex + 1] = green;
					row_pointer[yOffset][startDataIndex + 2] = blue;
				}
			}
			LOGD << "Successfully obtained dHash chunk at y=" << cinfo.next_scanline;
			// Now, calculate the dhash
			int widthOfDhashPixelChunk = framebufferWidth / widthOfdhashInput;
			uint8_t greyscaleRow[widthOfDhashPixelChunk];
			for(int dhashPixelChunk = 0; dhashPixelChunk < widthOfDhashPixelChunk; dhashPixelChunk++) {
				int pixelSumsR = 0;
				int pixelSumsG = 0;
				int pixelSumsB = 0;
				for(int relativeX = 0; relativeX < widthOfdhashInput; relativeX++) {
					for(int relativeY = 0; relativeY < heightOfdhashInput; relativeY++) {
						uint16_t xDataIndex = ((dhashPixelChunk * widthOfdhashInput) + relativeX) * jpegBytesPerPixel;

						// Apparently, it's better to use squaring in this situation, but oh well
						// https://sighack.com/post/averaging-rgb-colors-the-right-way
						pixelSumsR += row_pointer[relativeY][xDataIndex];
						pixelSumsG += row_pointer[relativeY][xDataIndex + 1];
						pixelSumsB += row_pointer[relativeY][xDataIndex + 2];
					}
				}
				int pixelAveragesR = pixelSumsR / (widthOfdhashInput * heightOfdhashInput);
				int pixelAveragesG = pixelSumsG / (widthOfdhashInput * heightOfdhashInput);
				int pixelAveragesB = pixelSumsB / (widthOfdhashInput * heightOfdhashInput);
				// Get greyscale value with a simple formula
				greyscaleRow[dhashPixelChunk] = (pixelAveragesR + pixelAveragesG + pixelAveragesB) / 3;
				// Don't use the first row yet
				if(dhashPixelChunk != 0) {
					// Use the comparison to set the bit
					bool isLeftBrighter = greyscaleRow[dhashPixelChunk - 1] < greyscaleRow[dhashPixelChunk];
					SET_BIT(dhash[indexByteDhash], isLeftBrighter, indexBitDhash);
					if(indexBitDhash == 7) {
						// Loop around
						indexBitDhash = 0;
						indexByteDhash++;
					} else {
						indexBitDhash++;
					}
				}
			}

			// Now, write the JPEG
			int scanlinesActuallyWritten = jpeg_write_scanlines(&cinfo, row_pointer, heightOfdhashInput);
			if(scanlinesActuallyWritten != heightOfdhashInput) {
				// Un oh
				LOGD << "Scanlines wrong in JPEG";
			}
			for(int x = 0; x < framebufferWidth; x++) {
				uint16_t y = cinfo.next_scanline;
				// Get the index
				// https://github.com/averne/dvdnx/blob/master/src/screen.hpp#L74
				// Swizzling pattern:
				//    y6,y5,y4,y3,y2,y1,y0,x7,x6,x5,x4,x3,x2,x1,x0
				// -> x7,x6,x5,y6,y5,y4,y3,x4,y2,y1,x3,y0,x2,x1,x0
				// Bits x0-4 and y0-6 are from memory layout spec (see TRM 20.1.2 - Block Linear) and libnx hardcoded values
				constexpr uint32_t x_mask = (__builtin_ctz(framebufferWidth) - 1) << 5;
				const uint32_t swizzled_x = ((x & x_mask) * 128) + ((x & 0b00010000) * 8) + ((x & 0b00001000) * 2) + (x & 0b00000111);
				const uint32_t swizzled_y = ((y & 0b1111000) * 32) + ((y & 0b0000110) * 16) + ((y & 0b0000001) * 8);
				uint32_t index            = swizzled_x + swizzled_y;

				// Laid out like RGBA
				uint8_t colorParts[4];
				svcReadDebugProcessMemory(&colorParts, VIdbg, initialPointer + index, sizeof(colorParts));

				// Set each value into the color data for this section of the JPEG
				uint16_t startDataIndex = x * jpegBytesPerPixel;

				uint8_t red   = colorParts[0];
				uint8_t green = colorParts[1];
				uint8_t blue  = colorParts[2];

				row_pointer[0][startDataIndex]     = red;
				row_pointer[0][startDataIndex + 1] = green;
				row_pointer[0][startDataIndex + 2] = blue;
			}
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		LOGD << "Successfully wrote JPEG";
		svcCloseHandle(VIdbg);

		jpeg_finish_compress(&cinfo);

		// Now that JPEG and dhash are done, send them both back
		hash->assign(convertToHexString(dhash, sizeOfDhash));
		// jpegBuffer->resize(jpegSize);
		// memcpy(jpegBuffer->data(), jpegBuf, jpegSize);

		jpeg_destroy_compress(&cinfo);
		fclose(outfile);

		// Read all data into memory
		hash->assign(convertToHexString(dhash, sizeOfDhash));

		jpegBuffer->clear();
		std::ifstream file(tempScreenshotName, std::ios::binary);
		std::copy(std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>(), std::back_inserter(*jpegBuffer));
		file.close();
		// remove(tempScreenshotName);
	} else {
		jpeg_destroy_compress(&cinfo);
		svcCloseHandle(VIdbg);
		fatalThrow(rc);
	}
	*/

}

std::string ScreenshotHandler::convertToHexString(uint8_t* data, uint16_t size) {
	// https://codereview.stackexchange.com/a/78539
	constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	std::string s(size * 2, ' ');
	for(int i = 0; i < size; ++i) {
		s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[data[i] & 0x0F];
	}
	return s;
}

ScreenshotHandler::~ScreenshotHandler() {
	free(row_pointer[0]);
}
