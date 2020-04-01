#include <map>
#include <plog/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <switch.h>
#include <vector>

#include "controller.hpp"
#include "mainLoopHandler.hpp"

extern "C" {
// Sysmodules should not use applet*.
u32 __nx_applet_type = AppletType_None;

// Adjust size as needed.
#define INNER_HEAP_SIZE 0x80000
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char nx_inner_heap[INNER_HEAP_SIZE];

void __libnx_init_time(void);
void __libnx_initheap(void);
void __appInit(void);
void __appExit(void);
}

void __libnx_initheap(void) {
	void* addr  = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	// Newlib
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

// Init/exit services, update as needed.
void __attribute__((weak)) __appInit(void) {
	Result rc;

	// Initialize default services.
	rc = smInitialize();
	if(R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

	rc = setsysInitialize();
	if(R_SUCCEEDED(rc)) {
		SetSysFirmwareVersion fw;
		rc = setsysGetFirmwareVersion(&fw);
		if(R_SUCCEEDED(rc))
			hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
		setsysExit();
	}

	// FS
	rc = fsInitialize();
	if(R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

	fsdevMountSdmc();

	// VI
	rc = viInitialize(ViServiceType_Manager);
	if(R_FAILED(rc))
		fatalThrow(rc);

	// HID
	rc = hidInitialize();
	if(R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

	// PMDMNT
	rc = pmdmntInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);

	rc = hiddbgInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);

	// vsync
	rc = setInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);

	// NS
	rc = nsInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);

	// PMINFO
	rc = pminfoInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);

	// Sockets
	constexpr SocketInitConfig sockConf = {
		.bsdsockets_version = 1,

		.tcp_tx_buf_size     = 0x800,
		.tcp_rx_buf_size     = 0x1000,
		.tcp_tx_buf_max_size = 0x2EE0,
		.tcp_rx_buf_max_size = 0x2EE0,

		.udp_tx_buf_size = 0x0,
		.udp_rx_buf_size = 0x0,

		.sb_efficiency = 4,
	};

	// sockets
	rc = socketInitialize(&sockConf);
	if(R_FAILED(rc)) {
		fatalThrow(rc);
	}
}

void __attribute__((weak)) userAppExit(void);

void __attribute__((weak)) __appExit(void) {
	socketExit();
	pminfoExit();
	nsExit();
	setExit();
	hiddbgExit();
	pmdmntExit();
	hidExit();
	viExit();
	fsdevUnmountAll();
	fsExit();

	smExit();
}

// Main program entrypoint
int main(int argc, char* argv[]) {
	remove("/NX-TAS-PC.log");
	plog::init(plog::debug, "/NX-TAS-PC.log");
	LOGD << "Started logging";

	// Sleep thread for a test
	// Gives me 15 seconds to turn off on reboot
	svcSleepThread((s64)1000000000 * 15);

	MainLoop mainLoop;

	while(true) {
		mainLoop.mainLoopHandler();
	}

	return 0;
}
