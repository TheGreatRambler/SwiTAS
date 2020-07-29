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