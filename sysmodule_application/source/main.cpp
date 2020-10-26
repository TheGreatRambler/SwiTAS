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
#define INNER_HEAP_SIZE 0xA00000
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

	// Account service, to get account name
	rc = accountInitialize(AccountServiceType_System);
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
	accountExit();
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
std::string logPath = HELPERS::getExecutableDir() + "/SwiTAS_log.txt";
remove(logPath.c_str());
plog::init(plog::debug, logPath.c_str());
LOGD << "Started logging";

MainLoop mainLoop;

DLL_EXPORT void startPlugin(void* wrapperInstance) {
	mainLoop.getYuzuSyscalls()->setYuzuInstance(wrapperInstance);
}

// Possibly pass delta
DLL_EXPORT void onMainLoop() {
	mainLoop.mainLoopHandler();
}

// clang-format off
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), meta_free)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_speedmode)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_frameadvance)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_pause)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_unpause)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_framecount)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_fps)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_emulating)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_romname)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getprogramid)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getprocessid)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getheapstart)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getheapsize)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getmainstart)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getmainsize)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getstackstart)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_getstacksize)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), emu_log)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), memory_readbyterange)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), memory_writebyterange)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), debugger_getclockticks)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), debugger_getcputicks)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_read)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_set)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_readjoystick)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_setjoystick)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_readsixaxis)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_setsixaxis)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_enablejoypad)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_removealljoypads)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_setjoypadtype)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_isjoypadconnected)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_requeststateupdate)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_enablekeyboard)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_enablemouse)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_enabletouchscreen)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_iskeypressed)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_setkeypressed)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_iskeymodifierpressed)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_setkeymodifierpressed)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_ismousepressed)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_setmousepressed)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_getnumtouches)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_setnumtouches)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_readtouch)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_settouch)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_movemouse)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), joypad_readmouse)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), input_enableoutsideinput)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_getwidth)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_getheight)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_clearscreen)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_render)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_drawpixel)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_savescreenshotas)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_drawimage)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_popup)
DLL_EXPORT SET_YUZU_FUNC(mainLoop.getYuzuSyscalls(), gui_savescreenshotmemory)

DLL_EXPORT uint64_t get_plugin_interface_version() { return PLUGIN_INTERFACE_VERSION; }
// clang-format on
#endif