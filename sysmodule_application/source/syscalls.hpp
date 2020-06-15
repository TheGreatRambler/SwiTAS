#pragma once

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef YUZU
#include "dllFunctionDefinitions.hpp"
#endif

class Syscalls {
private:
public:
#ifdef YUZU
	YUZU_FUNC(emu_speedmode)
	YUZU_FUNC(emu_frameadvance)
	YUZU_FUNC(emu_pause)
	YUZU_FUNC(emu_unpause)
	YUZU_FUNC(emu_message)
	YUZU_FUNC(emu_framecount)
	YUZU_FUNC(emu_emulating)
// Etc...
#endif

	Syscalls();
};