#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <plog/Log.h>
#include <string>
#include <vector>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "controller.hpp"
#include "mainLoopHandler.hpp"

#ifdef __SWITCH__
extern "C" {
// Sysmodules should not use applet*.
u32 __nx_applet_type = AppletType_None;

// Adjust size as needed.
#define INNER_HEAP_SIZE 0x300000
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

	// Screenshot service
	rc = capsscInitialize();
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
	capsscExit();
	viExit();
	fsdevUnmountAll();
	fsExit();
	smExit();
}
#endif

// Main program entrypoint
#ifdef __SWITCH__
int main(int argc, char* argv[]) {
	remove("/SwiTAS.log");
	plog::init(plog::debug, "/SwiTAS.log");
	LOGD << "Started logging";

	MainLoop mainLoop;

	while(true) {
		mainLoop.mainLoopHandler();
	}

	LOGD << "Loop done";

	return 0;
}
#endif

// Leaving the possibility open for a standalone exe later
// http://www.equestionanswers.com/c/c-explicit-linking.php
// http://anadoxin.org/blog/control-over-symbol-exports-in-mingw-linker.html
#ifdef YUZU
MainLoop mainLoop;

DLL_EXPORT void startPlugin(void* wrapperInstance) {
	// Do anything here that needs to be done
	// Wrapper instance needs to be saved by MainLoop to pass back to yuzu
	// This is because cross-module function calls are c functions
}
#endif
