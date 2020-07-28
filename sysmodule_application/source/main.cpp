#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#ifdef __SWITCH__
#include <plog/Log.h>
#include <switch.h>
#endif

#ifdef YUZU
#include "dllFunctionDefinitions.hpp"
#endif

#include "controller.hpp"
#include "mainLoopHandler.hpp"

#ifdef __SWITCH__
extern "C" {
// Sysmodules should not use applet*.
u32 __nx_applet_type = AppletType_None;

// Adjust size as needed.
#define INNER_HEAP_SIZE 0xC00000
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

	// Sleep mode handler
	rc = pscmInitialize();
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
	pscmExit();
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
	remove("/SwiTAS_log.txt");
	plog::init(plog::debug, "/SwiTAS_log.txt");
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
// https://stackoverflow.com/a/13256146/9329945
// http://anadoxin.org/blog/control-over-symbol-exports-in-mingw-linker.html
#ifdef YUZU
MainLoop mainLoop;

DLL_EXPORT void startPlugin(void* wrapperInstance) {
	mainLoop.getYuzuSyscalls()->setYuzuInstance(wrapperInstance);
}

// Possibly pass delta
DLL_EXPORT void handleMainLoop() {
	mainLoop.mainLoopHandler();
}

// clang-format off
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_speedmode)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_frameadvance)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_pause)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_unpause)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_message)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_framecount)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_emulating)
// clang-format on
// Etc...
#endif

	// Code on Yuzu side later
	/*
	#ifdef _WIN32
	#include <Windows.h>
	#endif

	#ifdef __linux__
	#include<dlfcn.h>
	#endif

	#ifdef _WIN32
	std::string GetLastErrorAsString()
	{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if(errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
	}
	#endif

	std::string sharedLibraryPath = ...;
	std::string sharedLibraryDirectory = ...;

	#ifdef _WIN32
	DLL_DIRECTORY_COOKIE directoryHandle = AddDllDirectory(sharedLibraryDirectory.c_str());
	if (!directoryHandle) {
		LOG(GetLastErrorAsString());
		return;
	}

	HMODULE sharedLibHandle = LoadLibraryEx(sharedLibraryPath.c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	if (sharedLibHandle == NULL) {
		LOG(GetLastErrorAsString());
		return;
	}

	// Do things

	FreeLibrary(sharedLibHandle);

	if (!RemoveDllDirectory(directoryHandle)) {
		LOG(GetLastErrorAsString());
		return;
	}
	#endif

	#ifdef __linux__
	// Set `LD_LIBRARY_PATH` (`DYLD_LIBRARY_PATH` on OSX) to add the folder. Save original value

	void* sharedLibHandle = (void *)dlopen(sharedLibraryPath.c_str(), RTLD_LAZY);
	if (!sharedLibHandle) {
		LOG(std::string(dlerror()));
		return;
	}

	// Do things

	dlclose(sharedLibHandle);

	// Restore original value to the ld path
	#endif
	*/
