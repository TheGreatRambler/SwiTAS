#pragma once

#include <switch.h>

class GetScreenshot {
private:
	u64 VI_pid;
	// The Title Id of VI (The display manager)
	constexpr u64 VITitleId = 0x010000000000002D;
	Handle VIdbg;
	// Size of the framebuffer
	// Width * Height * Byte depth
	// 1280 * 720 * 4
	constexpr u64 framebufferSize = 3686400;

public:
	// The pointer to the shared framebuffer
	// Data is swizzled https://github.com/averne/dvdnx/blob/master/src/screen.hpp#L74
	u8* framebuffer;
	
	GetScreenshot() {
		// Get the PID from the Title ID
		pmdmntGetProcessId(&VIdbg, HM);
		// Create the framebuffer
		framebuffer = new u8[framebufferSize];
	}
	
	void get() {
		// Get the framebuffer as of now
		Result rc = svcDebugActiveProcess(&HMdbg, HM_pid);
		if (R_SUCCEEDED(rc)) {
			// https://cdn.discordapp.com/attachments/656873506629484546/656906056089206785/unknown.png
			// I dunno
		}
	}
	
	~GetScreenshot() {
		// Delete the buffer now that it is not needed
		delete framebuffer[];
	}
}
