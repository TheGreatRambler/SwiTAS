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

// ROM Library (handled differently since the games are bigger)

typedef uint8_t(rom_readbyteunsigned)(uint64_t address);
typedef int8_t(rom_readbytesigned)(uint64_t address);
typedef void(rom_writebyte)(uint64_t address, uint8_t byte);

// Memory Library

typedef uint8_t(memory_readbyteunsigned)(uint64_t address);
typedef uint8_t*(memory_readbyterange)(uint64_t address, uint64_t length);
typedef int8_t(memory_readbytesigned)(uint64_t address);
// memory.readword(int addressLow, [int addressHigh]) ignored
// memory.readwordunsigned(int addressLow, [int addressHigh]) ignored
// memory.readwordsigned(int addressLow, [int addressHigh]) ignored
typedef void(memory_writebyte)(uint64_t address, uint8_t byte);
// int memory.getregister(cpuregistername) ignored, believe the name is different
// memory.setregister(string cpuregistername, int value) ignored, same as above
// memory.register(int address, [int size,] function func) ignored
// memory.registerwrite(int address, [int size,] function func) ignored
// memory.registerexec(int address, [int size,] function func) ignored, talk to Shadow
// memory.registerrun(int address, [int size,] function func) ignored, same as above
// memory.registerexecute(int address, [int size,] function func) ignored, same as above

// Debugger Library

typedef void(debugger_hitbreakpoint)();
typedef uint64_t(debugger_getcyclescount)();
typedef uint64_t(debugger_getinstructionscount)();
typedef void(debugger_resetcyclescount)();
typedef void(debugger_resetinstructionscount)();

// Joypad Library (Modified, based on libnx standards)

typedef uint64_t(joypad_read)(uint8_t player);
typedef uint64_t(joypad_immediate)(uint8_t player);
// table joypad.getdown(int player) ignored
// table joypad.readdown(int player) ignored
// table joypad.getup(int player) ignored
// table joypad.readup(int player) ignored
typedef void(joypad_set)(uint8_t player, uint64_t input);

// TODO add joystick and accel and gyro

// Input Library

// table input.get() ignored
// table input.read() ignored
typedef char*(input_popup)();

// TODO add keyboard mechanism based on enums