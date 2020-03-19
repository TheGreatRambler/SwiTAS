#include "handle_savestates.hpp"

SavestateHandler::SavestateHandler() {
	// Get the PID from the Title ID
	pmdmntGetProcessId(&VI_pid, VITitleId);
	// Get the pointer to the main framebuffer in VI
	Result rc = svcDebugActiveProcess(&VIdbg, VI_pid);
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
					break;
				}
			} else if(info.type == MemType_Reserved) {
				break;
			}
			addr += info.size;
		}
		svcCloseHandle(VIdbg);
	}
}

void SavestateHandler::setProjectFolder(std::string folder) {
	projectFolder = folder;
}

void SavestateHandler::createSavestateHookHere() {
	// The savestate hook holds basically just the framebuffer
	// For convinience, it holds a little bit of extra information
	// Like the frame number and the savefile index for SMO
	// Write BMP first
	std::string base = projectFolder + "/savestatehooks/" + std::to_string(currentFrame) + "/";
	FILE* bmpFile    = fopen((base + "framebuf.bmp").c_str(), "wb+");
	// Write the framebuf here
	BmpParser bmp;
	bmp.createBmp(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, bmpFile);
	writeFramebuffer(&bmp);
	// Close bmp file because writing is done
	fclose(bmpFile);
}

void SavestateHandler::setCurrentFrame(uint32_t frame) {
	currentFrame = frame;
}

void SavestateHandler::writeFramebuffer(BmpParser* bmp) {
	// This is a massive process with multiple steps
	// Writes to a BMP
	// This code is optimized in order to prevent the least amount of memory at any one time
	// The most memory that should ever be used is DATA_BUFFER plus perhaps a bit more
	// No compression is done because all the data would need to be uncompressed when reading the file
	// And the data would be in a buffer during that time
	Result rc = svcDebugActiveProcess(&VIdbg, VI_pid);
	if(R_SUCCEEDED(rc)) {
		// Depending on the type, it is offset differently
		// Currently only foreground
		uint64_t initialPointer = framebufferPointer + FramebufferType::FOREGROUND * framebufferSize;
		// Run swizzling ruutine
		// These loops are specifically written this way so they read data linearly
		for(uint16_t y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
			for(uint16_t x = 0; x < FRAMEBUFFER_WIDTH; x++) {
				// Get the index
				// https://github.com/averne/dvdnx/blob/master/src/screen.hpp#L74
				// Swizzling pattern:
				//    y6,y5,y4,y3,y2,y1,y0,x7,x6,x5,x4,x3,x2,x1,x0
				// -> x7,x6,x5,y6,y5,y4,y3,x4,y2,y1,x3,y0,x2,x1,x0
				// Bits x0-4 and y0-6 are from memory layout spec (see TRM 20.1.2 - Block Linear) and libnx hardcoded values
				constexpr uint32_t x_mask = (__builtin_ctz(FRAMEBUFFER_WIDTH) - 1) << 5;
				const uint32_t swizzled_x = ((x & x_mask) * 128) + ((x & 0b00010000) * 8) + ((x & 0b00001000) * 2) + (x & 0b00000111);
				const uint32_t swizzled_y = ((y & 0b1111000) * 32) + ((y & 0b0000110) * 16) + ((y & 0b0000001) * 8);
				uint32_t index            = swizzled_x + swizzled_y;
				// Set the pixel in one framebuffer to that of the other
				// From what I can tell, the data is in RGBA8888 format, so a pixel is 4 bytes
				// Endianness may be a massic problem here, but I don't know
				// Write directly to the file
				uint32_t thisColor;
				svcReadDebugProcessMemory(&thisColor, VIdbg, initialPointer + index, sizeof(thisColor));
				// Set the value in the bmp
				bmp->setColorValue(x, y, thisColor);
			}
		}
		// Undebug now because file writing is done
		svcCloseHandle(VIdbg);
	}
}

SavestateHandler::~SavestateHandler() { }