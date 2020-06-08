#pragma once

// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html

#include <cstdio>

// Emu library

// emu.poweron() ignored
// emu.softreset() ignored
typedef void(emu_speedmode)(char* mode);
typedef void(emu_frameadvance)();
typedef void(emu_pause)();
typedef void(emu_unpause)();
// emu.exec_count(int count, function func) ignored
// emu.exec_time(int time, function func) ignored
// emu.setrenderplanes(bool sprites, bool background) ignored
typedef void(emu_message)(char* mode);
typedef int(emu_framecount)();
// int emu.lagcount() ignored
// bool emu.lagged()
// emu.setlagflag(bool value) ignored
typedef bool(emu_emulating)();
typedef bool(emu_paused)();
// bool emu.readonly() ignored
// emu.setreadonly(bool state) ignored
typedef char*(emu_getdir)();
typedef void(emu_loadrom)(char* filename);
// emu.registerbefore(function func) handled outside of Yuzu
// emu.registerafter(function func) handled outside of Yuzu
// emu.registerexit(function func) handled outside of Yuzu
// bool emu.addgamegenie(string str) ignored
// bool emu.delgamegenie(string str) ignored
typedef void(emu_print)(char* mode);
typedef uint8_t*(emu_getscreenpixel)(int x, int y, bool getemuscreen);

// ROM Library