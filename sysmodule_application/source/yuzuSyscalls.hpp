#pragma once

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef YUZU
#include "dllFunctionDefinitions.hpp"
#endif

class Syscalls {
private:
	void* yuzuInstance;

public:
#ifdef YUZU
	YUZU_FUNC(emu_speedmode)
	YUZU_FUNC(emu_frameadvance)
	YUZU_FUNC(emu_pause)
	YUZU_FUNC(emu_unpause)
	YUZU_FUNC(emu_message)
	YUZU_FUNC(emu_framecount)
	YUZU_FUNC(emu_emulating)
	YUZU_FUNC(emu_isromopened)
	YUZU_FUNC(emu_romname)
	YUZU_FUNC(emu_getprogramid)
	YUZU_FUNC(emu_getprocessid)
	YUZU_FUNC(emu_getheapstart)
	YUZU_FUNC(emu_getmainstart)
	YUZU_FUNC(emu_log)
	YUZU_FUNC(joypad_getnumjoypads)
	YUZU_FUNC(joypad_setnumjoypads)
	YUZU_FUNC(joypad_addjoypad)
	YUZU_FUNC(emu_getscreenjpeg)
	YUZU_FUNC(joypad_readjoystick)
	YUZU_FUNC(joypad_read)

// Etc...
#endif

	Syscalls();

	void setYuzuInstance(void* instance) {
		yuzuInstance = instance;
	}

	void* getYuzuInstance() {
		return yuzuInstance;
	}
};